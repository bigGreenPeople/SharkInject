//
// Created by Administrator on 2021/5/3.
//
#pragma once

#ifndef ANDROIDINJECT_LIB_NAME_H

#define ANDROIDINJECT_LIB_NAME_H

#if defined(__aarch64__) || defined(__x86_64__)
//const char *libc_path = "/system/lib64/libc.so";
//const char *linker_path = "/system/bin/linker64";
//const char *libdl_path = "/system/lib64/libdl.so";

const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";
const char *libdl_path = "/system/lib/libdl.so";
#else
const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";
const char *libdl_path = "/system/lib/libdl.so";
#endif
#endif //ANDROIDINJECT_LIB_NAME_H
