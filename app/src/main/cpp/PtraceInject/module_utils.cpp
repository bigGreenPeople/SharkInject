#include <asm/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <asm/unistd.h>
#include "PrintLog.h"

#define  MAX_PATH 0x100

/*************************************************
 *  Description:    在指定进程中搜索对应模块的基址
 *  Input:          pid表示远程进程的ID，若为-1表示自身进程，ModuleName表示要搜索的模块的名称
 *  Return:         返回0表示获取模块基址失败，返回非0为要搜索的模块基址
 * **********************************************/
void *get_module_base_addr(pid_t pid, const char *ModuleName) {
    FILE *fp = NULL;
    long ModuleBaseAddr = 0;
    char szFileName[50] = {0};
    char szMapFileLine[1024] = {0};

    // 读取"/proc/pid/maps"可以获得该进程加载的模块
    if (pid < 0) {
        //  枚举自身进程模块
        snprintf(szFileName, sizeof(szFileName), "/proc/self/maps");
    } else {
        snprintf(szFileName, sizeof(szFileName), "/proc/%d/maps", pid);
    }

    fp = fopen(szFileName, "r");

    if (fp != NULL) {
        while (fgets(szMapFileLine, sizeof(szMapFileLine), fp)) {
            if (strstr(szMapFileLine, ModuleName)) {
                char *Addr = strtok(szMapFileLine, "-");
                ModuleBaseAddr = strtoul(Addr, NULL, 16);

                if (ModuleBaseAddr == 0x8000)
                    ModuleBaseAddr = 0;

                break;
            }
        }

        fclose(fp);
    }

    return (void *) ModuleBaseAddr;
}

/*************************************************
 *  Description:    获取远程进程与本进程都加载的模块中函数的地址
 *  Input:          pid表示远程进程的ID，ModuleName表示模块名称，LocalFuncAddr表示本地进程中该函数的地址
 *  Return:         返回远程进程中对应函数的地址
 * ***********************************************/
void *get_remote_func_addr(pid_t pid, const char *ModuleName, void *LocalFuncAddr) {
    void *LocalModuleAddr, *RemoteModuleAddr, *RemoteFuncAddr;

    LocalModuleAddr = get_module_base_addr(-1, ModuleName);
    RemoteModuleAddr = get_module_base_addr(pid, ModuleName);

    RemoteFuncAddr = (void *) ((uintptr_t) LocalFuncAddr - (uintptr_t) LocalModuleAddr +
                               (uintptr_t) RemoteModuleAddr);

    LOGI("[get_remote_func_addr] lmod=0x%lX, rmod=0x%lX, lfunc=0x%lX, rfunc=0x%lX",
         LocalModuleAddr, RemoteModuleAddr, LocalFuncAddr, RemoteFuncAddr);
    return RemoteFuncAddr;
}

/*************************************************
  Description:    通过进程名称定位到进程的PID
  Input:          process_name为要定位的进程名称
  Output:         无
  Return:         返回定位到的进程PID，若为-1，表示定位失败
  Others:         无
*************************************************/
pid_t find_pid_by_name(const char *process_name) {
    int dirid = 0;
    pid_t pid = -1;
    FILE *fp = NULL;
    char filename[MAX_PATH] = {0};
    char cmdline[MAX_PATH] = {0};

    struct dirent *entry = NULL;

    if (process_name == NULL)
        return -1;

    DIR *dir = opendir("/proc");
    if (dir == NULL)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        dirid = atoi(entry->d_name);
        if (dirid != 0) {
            snprintf(filename, MAX_PATH, "/proc/%d/cmdline", dirid);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                if (strncmp(process_name, cmdline, strlen(process_name)) == 0) {
                    pid = dirid;
                    LOGI("find cmdline: %s", cmdline);
                    break;
                }
            }
        }
    }

    closedir(dir);
    return pid;
}

long freespaceaddr(pid_t pid) {
    FILE *fp;
    char filename[50];
    char line[850];
    long addr;
    char str[20];
    char perms[5];
    sprintf(filename, "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    if (fp == NULL) {
        LOGE("open file %s failed, err:%s", filename, strerror(errno));
        exit(1);
    }

    while (fgets(line, 850, fp) != NULL) {
        sscanf(line, "%lx-%*lx %s %*s %s %*d", &addr, perms, str);

        if (strstr(perms, "x") != NULL) {
            break;
        }
    }
    fclose(fp);
    return addr;
}


#define INTEL_RET_INSTRUCTION 0xc3

unsigned char *findRet(void *endAddr) {
    unsigned char *retInstAddr = (unsigned char *) endAddr;
    while (*retInstAddr != INTEL_RET_INSTRUCTION) {
        retInstAddr--;
    }
    return retInstAddr;
}

// SeLinux的强制实施，使得dlopen()外部的so动态库有可能会失败返回。
// 关闭SeLinux setenforce 0; getenforce
// SeLinux会检测so动态库的label标签与权限是否满足可加载的要求，不满足就会无情的拒绝！
// 为了解决这个问题，需要调用setxattr()修改so的属性信息。
// chcon -v u:object_r:system_file:s0 /data/local/tmp/libInjectModule.so
//    setxattr(process_inject.libname, "u:object_r:system_file:s0");

/*int setxattr(const char *path, const char *value) {
    if ((access("/sys/fs/selinux", F_OK)) != -1) {
        return 0;
    }
    return syscall(__NR_setxattr, path, "security.selinux", value, strlen(value), 0);
}*/