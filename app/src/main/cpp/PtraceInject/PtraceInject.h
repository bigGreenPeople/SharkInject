//
// Created by Rose on 2020/2/18.
//

#ifndef INJECT_PTRACEINJECT_H
#define INJECT_PTRACEINJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>

int inject_remote_process(pid_t pid, char *LibPath, char *FunctionName,
                          char * parameter, long NumParameter);
int test(int argc, char *argv[]);

#endif //INJECT_PTRACEINJECT_H
