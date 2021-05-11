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
bool getPidByName(pid_t *pid, char *task_name);
#endif //ANDROIDINJECT_MODULE_UTILS_H
