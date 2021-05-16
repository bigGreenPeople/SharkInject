//
// Created by Administrator on 2021/5/3.
//

#ifndef ANDROIDINJECT_MODULE_UTILS_H
#define ANDROIDINJECT_MODULE_UTILS_H
void *get_module_base_addr(pid_t pid, const char *ModuleName);
pid_t find_pid_by_name(const char *process_name);
void *get_remote_func_addr(pid_t pid, const char *ModuleName, void *LocalFuncAddr);
long freespaceaddr(pid_t pid);
unsigned char *findRet(void *endAddr);
int setxattr(const char *path, const char *value);
bool get_pid_by_name(pid_t *pid, char *task_name);
void copy_file(char *source_path, char *destination_path);
void find_pkg_lib_path(char *pkgName, char *pkgLibPath);
void cp_lib(char * pkgName,char * libName);
void get_app_start_activity(char * pkg_name, char *start_activity_name);
void start_app(char * pkg_name);
#endif //ANDROIDINJECT_MODULE_UTILS_H
