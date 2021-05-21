#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>
#include "PrintLog.h"
#include "fake_dlfcn.h"
#include "load_dex.h"

pthread_t gThread;

#if defined(__aarch64__) || defined(__x86_64__)
const char *libandroid_runtime_path = "/system/lib64/libandroid_runtime.so";
#else
const char *libandroid_runtime_path = "/system/lib/libandroid_runtime.so";
#endif

/**
 * 这种方式是将我们的插件dex加载进内存后，在添加到app的ClassLoader中
 * 这样在app中是可以看到我们的代码的
 * @param env
 * @param jarpath
 */
void load_dex_and_run2(JNIEnv *env, const char *jarpath) {

    //使用DexFile.loadDex加载dex
    const char *pkgName = "com.shark.nougat";

    jobject dexObject = LoadDex(env, jarpath, pkgName);

    //获得app的PathClassLoader
    jobject appPathClassLoader = getClassLoader(env);
    //将我们加载的dex 放入到app的PathClassLoader的pathList;的dexElements;中
    //如此在app中就能直接加载到我们的dex中的类了
    makeDexElements(env, appPathClassLoader, dexObject);

    //找到我们的入口类
    const char *targetClass = "com/app/service/Entry";

    jclass Inject = myFindClass(env, targetClass, dexObject);

    //下面这一段就是去调用我们的入口类了
    jmethodID main = env->GetStaticMethodID(Inject, "onLoad",
                                            "(Ldalvik/system/PathClassLoader;Ljava/lang/String;Z)V");

    if (ClearException(env)) {
        LOGE("find Inject class Entry jmethodId failed");
        return;
    }

    //inject_flag is only used for art
    jboolean inject_flag = false;
    env->CallStaticVoidMethod(Inject, main, appPathClassLoader, env->NewStringUTF(pkgName),
                              inject_flag);
    if (ClearException(env)) {
        LOGE("call Entry method failed");
        return;
    }
}

/**
 *
 * 下面这种方式是直接自己创建一个ClassLoader 这样创建的ClassLoader在App的加载器中是找不到我们的注入代码的
 * @param env
 * @param jarpath
 */
void load_dex_and_run(JNIEnv *env, const char *jarpath) {
    jobject appPathClassLoader = getClassLoader(env);
    //获取当前目录
    char current_absolute_path[4096] ="/data/local/tmp";

    jobject myClassLoader = createNewClassLoader(env, jarpath, current_absolute_path);
    LOGI("myClassLoader 0x%p\n", myClassLoader);
    if (NULL != myClassLoader) {
        jclass entry_class = findClassFromLoader(env, myClassLoader, "com.app.service.Entry");

        if (NULL != entry_class) {
            LOGI("Entry Class 0x%p\n", entry_class);
            //"(Ldalvik/system/PathClassLoader;Ljava/lang/String;Z)V"

            const char *entryName = "onLoad";
            jmethodID entry_method = env->GetStaticMethodID(entry_class, entryName,
                                                            "(Ldalvik/system/PathClassLoader;Ljava/lang/String;Z)V");

            if (NULL != entry_method) {
                jboolean inject_flag = false;
                const char *pkgName = "com.shark.initapp";

                env->CallStaticVoidMethod(entry_class, entry_method, appPathClassLoader,
                                          env->NewStringUTF(pkgName), inject_flag);
            }
        }
    }
}

int _clientInit(const char *jarpath) {
    JNIEnv *testenv = NULL;
    void *handle;
    //依靠libandroid_runtime.so 找到JavaVM
    handle = dlopen_ex(libandroid_runtime_path, RTLD_NOW);
    LOGI("fake_dlopen for libandroid_runtime.so returned %p\n", handle);
    void *pVM = dlsym_ex(handle, "_ZN7android14AndroidRuntime7mJavaVME");

    JavaVM *javaVM = (JavaVM *) *(void **) pVM;
    LOGI("use mJavaVM returned %p\n", javaVM);
    if (javaVM) {
        jint result = javaVM->AttachCurrentThread(&testenv, 0);
        if ((result == JNI_OK) && (testenv != NULL)) {
            LOGI("attach ok. clientInit JavaVM : 0x%p, JNIEnv : 0x%p\n", javaVM, testenv);
            load_dex_and_run(testenv, jarpath);
            javaVM->DetachCurrentThread();
            LOGI("DetachCurrentThread all finished!");
        } else {
            LOGE("NOTE: attach of thread failed\n");
            return -1;
        }
    }

    return 0;
}

extern "C" __attribute__((visibility("default"))) int entry(char *so_parameter) {
    LOGE("[InjectModule] Inject_entry Func is called\n");
    pthread_create(&gThread, NULL, (void *(*)(void *)) _clientInit, (void *) so_parameter);

    return 0;
}