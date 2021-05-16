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
#include <sys/stat.h>
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
    //[get_remote_func_addr] lmod=0x752805B000, rmod=0x0,          lfunc=0x752805C0C0, rfunc=0x10C0
    //[get_remote_func_addr] lmod=0x7CBB472000, rmod=0x0,          lfunc=0x7CBB4730EC, rfunc=0x10EC
    //[get_remote_func_addr] lmod=0x72921C6000, rmod=0x748CD89000, lfunc=0x72921C70C0, rfunc=0x748CD8A0C0
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

bool get_pid_by_name(pid_t *pid, char *task_name) {
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char filepath[50];
    char cur_task_name[50];
    char buf[1024];

    dir = opendir("/proc");
    if (NULL != dir) {
        while ((ptr = readdir(dir)) != NULL) //循环读取/proc下的每一个文件/文件夹
        {
            //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);//生成要读取的文件的路径
            fp = fopen(filepath, "r");
            if (NULL != fp) {
                if (fgets(buf, 1024 - 1, fp) == NULL) {
                    fclose(fp);
                    continue;
                }
                sscanf(buf, "%s", cur_task_name);
                //如果文件内容满足要求则打印路径的名字（即进程的PID）
                if (strstr(task_name, cur_task_name)) {
//                    sscanf(ptr->d_name, "%d", pid);
                    printf("d_name %s\n", ptr->d_name);
                    *pid = atoi(ptr->d_name);
                    return true;

                }
                fclose(fp);
            }
        }
        closedir(dir);
    }

    return false;
}


void copy_file(char *source_path, char *destination_path) {
    char buffer[1024];
    FILE *in, *out;
    if ((in = fopen(source_path, "r")) == NULL) {
        printf("源文件打开失败！\n");
        perror("fopen");
        exit(1);
    }
    if ((out = fopen(destination_path, "w")) == NULL) {
        printf("目标文件创建失败！\n");
        perror("fopen");
        exit(1);
    }
    int len;
    while ((len = fread(buffer, 1, 1024, in)) > 0) {
        fwrite(buffer, 1, len, out);
    }
    fclose(out);
    fclose(in);
}

void find_pkg_lib_path(char *pkgName, char *pkgLibPath) {
    DIR *pDir;
    struct dirent *ent;
    int i = 0;
    char childpath[256];
    pDir = opendir("/data/app");
    memset(childpath, 0, sizeof(childpath));

    while ((ent = readdir(pDir)) != NULL) {
        if (ent->d_type & DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            if (strstr(ent->d_name, pkgName) != NULL) {
                strcat(pkgLibPath, "/data/app/");
                strcat(pkgLibPath, ent->d_name);
                strcat(pkgLibPath, "/lib");
                break;
            }

        }
    }
}

void cp_lib(char * pkgName,char * libName){
    //获取目标lib目录
    char pkgLibPath[256] = {0};
    find_pkg_lib_path(pkgName, pkgLibPath);
    //如果没有arm64则创建
    strcat(pkgLibPath, "/arm/");
    printf("pkgLibPath %s \n", pkgLibPath);

    if ((access(pkgLibPath, F_OK)) == -1) {
        if (mkdir(pkgLibPath, 0755) == -1) {
            perror("mkdir");
            exit(-1);
        }
    }
    strcat(pkgLibPath, libName);
    //获取当前目录
    char current_absolute_path[4096] = {0};
    if (realpath("./", current_absolute_path)==NULL) {
        perror("realpath");
        exit(-1);
    }
    //得到lib的路径
    char libPath[500] = {};
    strcat(current_absolute_path, "/");
    strcat(libPath,current_absolute_path);
    strcat(libPath,libName);
    printf("pkgLibPath %s \n", pkgLibPath);
    printf("libPath %s \n", libPath);
    copy_file(libPath,pkgLibPath);
}

/**
 *  获取应用的android.intent.action.MAIN
 * @param pkg_name 获取的应用包名
 * @param start_activity_name out 存放启动的main activity
 */
void get_app_start_activity(char * pkg_name, char *start_activity_name) {
    char cmdstring[1024] = "dumpsys package ";
    char cmd_string[1024] = {0};
    char temp_file[] = "tmp_XXXXXX";

    strcat(cmdstring,pkg_name);
    int fd;

    if ((fd = mkstemp(temp_file)) == -1) {
        printf("create tmp file failed.\n");
    }

    sprintf(cmd_string, "%s > %s", cmdstring, temp_file);
    system(cmd_string);

    FILE *fp = fdopen(fd, "r");
    if (fp == NULL) {
        printf("can not load file!");
        return;
    }
    char line[1024];
    while (!feof(fp)) {
        fgets(line, 1024, fp);
        if (strstr(line, "android.intent.action.MAIN")) {
            fgets(line, 1024, fp);
            char *p;
            //选取第二个
            int index = 1;
            p = strtok(line, " ");
            while (p) {
                if (index == 2) {
                    strcpy(start_activity_name, p);
                }
                index++;
                p = strtok(NULL, " ");
            }
            break;
        }
    }
    fclose(fp);
    unlink(temp_file);
    return;
}

/**
 * 启动app
 * @param pkg_name
 */
void start_app(char * pkg_name){
    char start_activity_name[1024] = {0};
    get_app_start_activity(pkg_name, start_activity_name);
    printf("%s\n", start_activity_name);

    char start_cmd[1024] = "am start ";
    strcat(start_cmd, start_activity_name);
    system(start_cmd);
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