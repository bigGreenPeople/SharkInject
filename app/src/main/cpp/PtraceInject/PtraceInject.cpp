#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <elf.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include "PtraceInject.h"
#include "PrintLog.h"
#include "ptrace_utils.h"
#include "module_utils.h"

/*************************************************
 *  通过远程直接调用dlopen\dlsym的方法ptrace注入so模块到远程进程中
 *  Input:          pid表示远程进程的ID，LibPath为被远程注入的so模块路径，FunctionName为远程注入的模块后调用的函数
 *                  FuncParameter指向被远程调用函数的参数（若传递字符串，需要先将字符串写入到远程进程空间中），
 *                  NumParameter为参数的个数
 *  Return:         返回0表示注入成功，返回-1表示失败
 * ***********************************************/
int inject_remote_process(pid_t pid, char *LibPath, char *FunctionName,
                          char *parameter, long NumParameter) {
    int iRet = -1;
    long parameters[6];
    //attach到目标进程
    if (ptrace_attach(pid) != 0) {
        return iRet;
    }

    do {
        //CurrentRegs 当前寄存器
        //OriginalRegs 保存注入前寄存器
        struct pt_regs CurrentRegs, OriginalRegs;

        if (ptrace_getregs(pid, &CurrentRegs) != 0) {
            break;
        }

        memcpy(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs));

        void *mmap_addr = get_mmap_address(pid);
        printf("mmap RemoteFuncAddr:0x%lx\n", (uintptr_t) mmap_addr);

        // mmap映射
        parameters[0] = 0;  // 设置为NULL表示让系统自动选择分配内存的地址
        parameters[1] = 0x3000; // 映射内存的大小
        parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // 表示映射内存区域可读可写可执行
        parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE; // 建立匿名映射
        parameters[4] = 0; //  若需要映射文件到内存中，则为文件的fd
        parameters[5] = 0; //文件映射偏移量

        if (ptrace_call(pid, (uintptr_t) mmap_addr, parameters, 6, &CurrentRegs) == -1) {
            printf("Call Remote mmap Func Failed, err:%s\n", strerror(errno));
            break;
        }

        printf("ptrace_call mmap success, return value=%lX, pc=%lX\n", ptrace_getret(&CurrentRegs),
             ptrace_getpc(&CurrentRegs));

        // 获取mmap函数执行后的返回值，也就是内存映射的起始地址
        void *RemoteMapMemoryAddr = (void *) ptrace_getret(&CurrentRegs);
        printf("Remote Process Map Memory Addr:0x%lx\n", (uintptr_t) RemoteMapMemoryAddr);


        // 分别获取dlopen、dlsym、dlclose等函数的地址
        void *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;
        dlopen_addr = get_dlopen_address(pid);
        dlsym_addr = get_dlsym_address(pid);
        dlclose_addr = get_dlclose_address(pid);
        dlerror_addr = get_dlerror_address(pid);
        // 将要加载的so库路径写入到远程进程内存空间中
        if (ptrace_writedata(pid, (uint8_t *) RemoteMapMemoryAddr, (uint8_t *) LibPath,
                             strlen(LibPath) + 1) == -1) {
            printf("Write LibPath:%s to RemoteProcess error\n", LibPath);
            break;
        }

        // 设置dlopen的参数,返回值为模块加载的地址
        // void *dlopen(const char *filename, int flag);
        parameters[0] = (uintptr_t) RemoteMapMemoryAddr;
        parameters[1] = RTLD_NOW | RTLD_GLOBAL;

        if (ptrace_call(pid, (uintptr_t) dlopen_addr, parameters, 2, &CurrentRegs) == -1) {
            printf("Call Remote dlopen Func Failed\n");
            break;
        }

        // RemoteModuleAddr为远程进程加载注入模块的地址
        void *RemoteModuleAddr = (void *) ptrace_getret(&CurrentRegs);
        printf("ptrace_call dlopen success, Remote Process load module Addr:0x%lx\n",
             (long) RemoteModuleAddr);

        if ((long) RemoteModuleAddr == 0x0)   // dlopen 错误
        {
            printf("dlopen error\n");
            if (ptrace_call(pid, (uintptr_t) dlerror_addr, parameters, 0, &CurrentRegs) == -1) {
                printf("Call Remote dlerror Func Failed\n");
                break;
            }
            char *Error = (char *) ptrace_getret(&CurrentRegs);
            char LocalErrorInfo[1024] = {0};
            ptrace_readdata(pid, (uint8_t *) Error, (uint8_t *) LocalErrorInfo, 1024);
            printf("dlopen error:%s\n", LocalErrorInfo);
            break;
        }

        // 将so库中需要调用的函数名称写入到远程进程内存空间中
        if (ptrace_writedata(pid, (uint8_t *) RemoteMapMemoryAddr + strlen(LibPath) + 2,
                             (uint8_t *) FunctionName, strlen(FunctionName) + 1) == -1) {
            printf("Write FunctionName:%s to RemoteProcess error\n", FunctionName);
            break;
        }

        // 设置dlsym的参数，返回值为远程进程内函数的地址
        // void *dlsym(void *handle, const char *symbol);
        parameters[0] = (uintptr_t) RemoteModuleAddr;
        parameters[1] = (uintptr_t) ((uint8_t *) RemoteMapMemoryAddr + strlen(LibPath) + 2);

        if (ptrace_call(pid, (uintptr_t) dlsym_addr, parameters, 2, &CurrentRegs) == -1) {
            printf("Call Remote dlsym Func Failed\n");
            break;
        }

        // RemoteModuleFuncAddr为远程进程空间内获取的函数地址
        void *RemoteModuleFuncAddr = (void *) ptrace_getret(&CurrentRegs);
        printf("ptrace_call dlsym success, Remote Process ModuleFunc Addr:0x%lx\n",
             (uintptr_t) RemoteModuleFuncAddr);

        //为调用的函数参数，拷贝字符串 (这里分配到0x200后面 一般上面的字符串不会超过)
        if (ptrace_writedata(pid, (uint8_t *) RemoteMapMemoryAddr + strlen(LibPath) + 2 +
                                  strlen(parameter) + 2,
                             (uint8_t *) parameter, strlen(parameter) + 1) == -1) {
            printf("Write param error\n");
            break;
        }
        parameters[0] = (uintptr_t) ((uint8_t *) RemoteMapMemoryAddr + strlen(LibPath) + 2 +
                                     strlen(parameter) + 2);

        //这里其实应该计算参数的大小 然后在分配内存空间
        //暂时传一个参数吧
        if (ptrace_call(pid, (uintptr_t) RemoteModuleFuncAddr, parameters, NumParameter,
                        &CurrentRegs) == -1) {
            printf("Call Remote injected Func Failed\n");
            break;
        }

        if (ptrace_setregs(pid, &OriginalRegs) == -1) {
            printf("Recover reges failed\n");
            break;
        }
        printf("Recover Regs Success\n");
        ptrace_getregs(pid, &CurrentRegs);
        if (memcmp(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs)) != 0) {
            printf("Set Regs Error\n");
        }
        iRet = 0;
    } while (false);

    ptrace_detach(pid);
    return iRet;
}

struct process_inject {
    pid_t pid;
    char lib_name[1024];
    char dex_name[1024];
    char *entry_fun;
} process_inject = {0, "", "", "entry"};


void init_lib() {
    //获取当前目录
    char current_absolute_path[4096] = {0};
    if (realpath("./", current_absolute_path) == NULL) {
        perror("realpath");
        exit(-1);
    }
    strcat(process_inject.lib_name, current_absolute_path);
    strcat(process_inject.lib_name, "/libLoadModule.so");

    strcat(process_inject.dex_name, current_absolute_path);
    strcat(process_inject.dex_name, "/classes.dex");

    //修改为默认从加载so的路径读取
//    cp_lib(pkgName,"libsandhook.so");
}

void handle_parameter(int argc, char *argv[]) {
    pid_t pid = 0;
    int index = 0;
    char *pkg_name = NULL;
    char *file_name = NULL;
    char *dex_name = NULL;
    bool start_app_flag = false;

    while (index < argc) {
        if (strcmp("-p", argv[index]) == 0) {
            if (index + 1 >= argc) {
                printf("Missing parameter -p\n");
                exit(-1);
            }
            index++;
            pid = atoi(argv[index]);
        }

        if (strcmp("-n", argv[index]) == 0) {
            if (index + 1 >= argc) {
                printf("Missing parameter -n\n");
                exit(-1);
            }
            index++;
            pkg_name = argv[index];

            if (start_app_flag) {
                start_app(pkg_name);
                sleep(1);
            }
        }

        if (strcmp("-so", argv[index]) == 0) {
            if (index + 1 >= argc) {
                printf("Missing parameter -f\n");
                exit(-1);
            }
            index++;
            file_name = argv[index];
        }

        if (strcmp("-f", argv[index]) == 0) { start_app_flag = true; }

        if (strcmp("-d", argv[index]) == 0) {
            if (index + 1 >= argc) {
                printf("Missing parameter -d\n");
                exit(-1);
            }
            index++;
            dex_name = argv[index];
        }
        index++;
    }

    if (pkg_name != NULL) {
        printf("pkg_name is %s\n", pkg_name);
        if (get_pid_by_name(&pid, pkg_name)) {
            printf("getPidByName pid is %d\n", pid);
        }
    }

    if (pid == 0) {
        printf("not found target\n");
        exit(0);
    }

    process_inject.pid = pid;

    if (dex_name != NULL) {
        printf("dex_name is %s\n", dex_name);
        strcpy(process_inject.dex_name, strdup(dex_name));
    }


    if (file_name != NULL) {
        printf("file_name is %s\n", file_name);
        strcpy(process_inject.lib_name, strdup(file_name));
    }
}

int test(int argc, char *argv[]) {
    //初始化dex so
    init_lib();
    //处理参数
    handle_parameter(argc, argv);

    return inject_remote_process(process_inject.pid, process_inject.lib_name,
                                 process_inject.entry_fun, process_inject.dex_name, 1);
}
