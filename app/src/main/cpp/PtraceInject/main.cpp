//
// Created by haoyuanli on 2020-3-24.
//
#include <stdio.h>
#include <stdlib.h>
#include "PrintLog.h"
#include "PtraceInject.h"

int main(int argc, char *argv[]) {
    LOGI("[+] start main\n");
    test(argc,argv);
    return 0;
}