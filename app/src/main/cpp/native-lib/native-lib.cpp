#include <jni.h>
#include <string>
#include <dlfcn.h>
#include "PrintLog.h"

typedef int (*FUNC_INJECT_ENTRY)();

int test_load_library() {
    char InjectModuleName[] = "/data/app/com.example.androidinject-waaMZzkROOdkpeuvRzkKAQ==/lib/arm64/libInjectModule.so";    // 注入模块全路径
    void *handle = dlopen(InjectModuleName, RTLD_LAZY);
    if (!handle) {
        LOGE("[%s](%d) dlopen %s error:%s", __FILE__, __LINE__, InjectModuleName, dlerror());
        return 0;
    }

    do {
        FUNC_INJECT_ENTRY entry_func = (FUNC_INJECT_ENTRY) dlsym(handle, "Inject_entry");
        if (NULL == entry_func) {
            LOGE("[%s](%d) dlsym %s error:%s", __FILE__, __LINE__, "Inject_entry", dlerror());
            break;
        }
        entry_func();
    } while (false);

    dlclose(handle);
    return 1;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_androidinject_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_androidinject_MainActivity_testload(
        JNIEnv *env,
        jobject /* this */) {
    return test_load_library();
}