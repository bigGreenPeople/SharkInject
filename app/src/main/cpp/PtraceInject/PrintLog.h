#ifndef _ANDROID_LOG_PRINT_H_
#define _ANDROID_LOG_PRINT_H_

#include <android/log.h>

//如果不想打印日志可以注释这行宏定义
#define IS_DEBUG
//如果宏定义了IS_DEBUG，那么下面就会宏定义下面这些日志打印函数
#ifdef IS_DEBUG

#define LOG_TAG ("SharkChilli")

#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__))

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO   , LOG_TAG, __VA_ARGS__))

#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN   , LOG_TAG, __VA_ARGS__))

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR  , LOG_TAG, __VA_ARGS__))

#else

#define LOGV(LOG_TAG, ...) NULL

#define LOGD(LOG_TAG, ...) NULL

#define LOGI(LOG_TAG, ...) NULL

#define LOGW(LOG_TAG, ...) NULL

#define LOGE(LOG_TAG, ...) NULL

#endif

#endif