#include <jni.h>
#include "android/log.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <dlfcn.h>
#define ANDROID_SMP 0
#include <alloca.h>
#include <sys/system_properties.h>
#include "load_dex.h"
#include "PrintLog.h"

static jobject g_orignalDex;
static jobject g_classLoader=0;


int ClearException(JNIEnv *jenv){
    JNIEnv  * test = NULL;
    jthrowable exception = jenv->ExceptionOccurred();
    if (exception != NULL) {
        jenv->ExceptionDescribe();
        jenv->ExceptionClear();
        return true;
    }
    return false;
}



int makeDexElements(JNIEnv* env, jobject classLoader, jobject dexFileobj)
{
    jclass PathClassLoader = env->GetObjectClass(classLoader);

    jclass BaseDexClassLoader = env->GetSuperclass(PathClassLoader);

    //get pathList fieldid
    jfieldID pathListid = env->GetFieldID(BaseDexClassLoader, "pathList", "Ldalvik/system/DexPathList;");
    jobject pathList = env->GetObjectField(classLoader, pathListid);

    //get DexPathList Class
    jclass DexPathListClass = env->GetObjectClass(pathList);
    //get dexElements fieldid
    jfieldID dexElementsid = env->GetFieldID(DexPathListClass, "dexElements", "[Ldalvik/system/DexPathList$Element;");

    //获取elements数组 get dexElement array value
    jobjectArray dexElement = static_cast<jobjectArray>(env->GetObjectField(pathList, dexElementsid));


    //获取数组的个数 get DexPathList$Element Class construction method and get a new DexPathList$Element object
    jint len = env->GetArrayLength(dexElement);
    LOGI( "original Element size:%d", len);


    jclass ElementClass = env->FindClass("dalvik/system/DexPathList$Element");// dalvik/system/DexPathList$Element
    jmethodID Elementinit = env->GetMethodID(ElementClass, "<init>", "(Ljava/io/File;ZLjava/io/File;Ldalvik/system/DexFile;)V");
    jboolean isDirectory = JNI_FALSE;

    /**
     * get origianl dex object
     */
    jobject originalDexElement=env->GetObjectArrayElement(dexElement,0);
    if(originalDexElement!=0)
    {
        jfieldID tmp=env->GetFieldID(ElementClass,"dexFile","Ldalvik/system/DexFile;");
        g_orignalDex=env->GetObjectField(originalDexElement,tmp);
        if(ClearException(env)){
            LOGI("get original DexObj faield");
        }
    }

    //创建一个新的dalvik/system/DexPathList$Element类 dexFileobj为新的dexFileobj
    jobject element_obj = env->NewObject(ElementClass, Elementinit, 0, isDirectory, 0, dexFileobj);

    //Get dexElement all values and add  add each value to the new array
    jobjectArray new_dexElement = env->NewObjectArray(len + 1, ElementClass, 0);
    for (int i = 0; i < len; ++i)
    {
        //将以前的Elements添加到这个新的new_dexElement数组
        env->SetObjectArrayElement(new_dexElement, i, env->GetObjectArrayElement(dexElement, i));
    }
    //将要加载的element_obj放在新数组的最后一个成员里
    env->SetObjectArrayElement(new_dexElement, len, element_obj);
    env->SetObjectField(pathList, dexElementsid, new_dexElement);
    LOGI("make complete");
    env->DeleteLocalRef(element_obj);
    env->DeleteLocalRef(ElementClass);
    env->DeleteLocalRef(dexElement);
    env->DeleteLocalRef(DexPathListClass);
    env->DeleteLocalRef(pathList);
    env->DeleteLocalRef(BaseDexClassLoader);
    env->DeleteLocalRef(PathClassLoader);
    return 1;
}



jobject getClassLoader(JNIEnv *jenv){
    //获取Loaders
    jclass clazzApplicationLoaders = jenv->FindClass("android/app/ApplicationLoaders");
    jthrowable exception = jenv->ExceptionOccurred();
    if (ClearException(jenv)) {
        LOGI("Exception","No class : %s", "android/app/ApplicationLoaders");
        return NULL;
    }
    jfieldID fieldApplicationLoaders = jenv->GetStaticFieldID(clazzApplicationLoaders,"gApplicationLoaders","Landroid/app/ApplicationLoaders;");
    if (ClearException(jenv)) {
        LOGI("Exception","No Static Field :%s","gApplicationLoaders");
        return NULL;
    }
    jobject objApplicationLoaders = jenv->GetStaticObjectField(clazzApplicationLoaders,fieldApplicationLoaders);
    if (ClearException(jenv)) {
        LOGI("Exception","GetStaticObjectField is failed [%s","gApplicationLoaders");
        return NULL;
    }
    //

    jfieldID fieldLoaders = jenv->GetFieldID(clazzApplicationLoaders,"mLoaders","Ljava/util/Map;");
    if (ClearException(jenv)) {
        fieldLoaders = jenv->GetFieldID(clazzApplicationLoaders,"mLoaders","Landroid/util/ArrayMap;");
        if(ClearException(jenv)){
            LOGI("Exception","No Field :%s","mLoaders");
            return NULL;
        }

    }

    jobject objLoaders = jenv->GetObjectField(objApplicationLoaders,fieldLoaders);
    if (ClearException(jenv)) {
        LOGI("Exception","No object :%s","mLoaders");
        return NULL;
    }
    //提取map中的values
    jclass clazzHashMap = jenv->GetObjectClass(objLoaders);
    jmethodID methodValues = jenv->GetMethodID(clazzHashMap,"values","()Ljava/util/Collection;");
    jobject values = jenv->CallObjectMethod(objLoaders,methodValues);

    jclass clazzValues = jenv->GetObjectClass(values);
    jmethodID methodToArray = jenv->GetMethodID(clazzValues,"toArray","()[Ljava/lang/Object;");
    if (ClearException(jenv)) {
        LOGI("Exception","No Method:%s","toArray");
        return NULL;
    }

    jobjectArray classLoaders = (jobjectArray)jenv->CallObjectMethod(values,methodToArray);
    if (ClearException(jenv)) {
        LOGI("Exception","CallObjectMethod failed :%s","toArray");
        return NULL;
    }

    int size = jenv->GetArrayLength(classLoaders);

    //classLoaders size always is 1 ???
    LOGI("classLoaders size:%d",size);

    for(int i = 0 ; i < size ; i ++){
        jobject classLoader = jenv->GetObjectArrayElement(classLoaders,i);
        g_classLoader=jenv->NewGlobalRef(classLoader);
        if(g_classLoader==NULL){
            LOGI("classLoader NewGlobalRef failed");
            return NULL;
        }
        jenv->DeleteLocalRef(classLoader);
        return g_classLoader;
    }

}

jclass loadCLass_plan_one(JNIEnv *jenv,const char *name,jobject dexObject)
{
    loadCLass_plan_one(jenv,name,dexObject);
    jclass DexFile=jenv->FindClass("dalvik/system/DexFile");
    jmethodID loadClass=jenv->GetMethodID(DexFile,"loadClass","(Ljava/lang/String;Ljava/lang/ClassLoader;)Ljava/lang/Class;");
    if(ClearException(jenv))
    {
        LOGI("find loadClass methodId failed");
        return 0;
    }
    //important dexObject.loadClass()
    jstring className=jenv->NewStringUTF(name);
    jclass tClazz = (jclass)jenv->CallObjectMethod(dexObject,loadClass,className,g_classLoader);
    if(ClearException(jenv))
    {
        LOGI("loadClass %s failed",name);
        return 0;
    }
    return tClazz;

}

//MultiDex
jclass loadCLass_plan_two(JNIEnv *jenv,const char *name)
{
    jstring className=jenv->NewStringUTF(name);
    jclass clazzCL = jenv->GetObjectClass(g_classLoader);
    jmethodID loadClass = jenv->GetMethodID(clazzCL,"loadClass","(Ljava/lang/String;)Ljava/lang/Class;");
    jclass tClazz = (jclass)jenv->CallObjectMethod(g_classLoader,loadClass,className);

    if(ClearException(jenv))
    {
        LOGI("loadClass %s failed",name);
        return 0;
    }
    return tClazz;
}

jclass findAppClass_test(JNIEnv *jenv,const char *name,jobject dexObject)
{
    //there hava 2 plan to loadClass
    // plan 1
    //jclass TargetClazz=loadCLass_plan_one(jenv,name,dexObject);

    // plan 2
    jclass TargetClazz=loadCLass_plan_two(jenv,name);
    if(TargetClazz!=0)
        LOGI("loadClass %s successful clazz:0x%x",name,TargetClazz);
    return TargetClazz;
}


jobject LoadDex(JNIEnv* jenv,const char* dexPath,const char* pKgName)
{
    jclass DexFile=jenv->FindClass("dalvik/system/DexFile");
    if(ClearException(jenv))
    {
        LOGI("find DexFile class failed");
        return 0;
    }
    jmethodID loadDex=jenv->GetStaticMethodID(DexFile,"loadDex","(Ljava/lang/String;Ljava/lang/String;I)Ldalvik/system/DexFile;");
    if(ClearException(jenv))
    {
        LOGI("find loadDex methodId failed");
        return 0;
    }
    //jstring inPath=jenv->NewStringUTF("/data/data/com.example.stromhooktest/legend.dex");
    jstring inPath=jenv->NewStringUTF(dexPath);

    char optPath[256]={0};
    strcat(optPath,"/data/data/");
    strcat(optPath,pKgName);
    strcat(optPath,"/hook.dat");
    LOGI("LoadDex optFile path:%s",optPath);
    jstring outPath=jenv->NewStringUTF(optPath);
    jobject dexObject=jenv->CallStaticObjectMethod(DexFile,loadDex,inPath,outPath,0);
    if(ClearException(jenv))
    {
        LOGI("call loadDex method failed");
        return 0;
    }
    return dexObject;
}


jclass myFindClass(JNIEnv* jenv,const char* targetClassName,jobject dexObj)
{
    //char* targetClassName="com/legend/demo/Inject";
    jclass clazzTarget = jenv->FindClass(targetClassName);
    if (ClearException(jenv)) {
        LOGI("ClassMethodHook[Can't find class:%s in bootclassloader",targetClassName);
        clazzTarget = findAppClass_test(jenv,targetClassName,dexObj);
        if(clazzTarget == NULL){
            LOGI("found class %s failed",targetClassName);
            return NULL;
        }
    }

    return clazzTarget;
}

jobject createNewClassLoader(JNIEnv* env, const char *jarpath,char * nativepath){
    jclass clzPathClassLoader = env->FindClass("dalvik/system/PathClassLoader");
//    LOGI("java/lang/ClassLoader 0x%p\n", clzClassLoader);
    jmethodID mdinitPathCL = env->GetMethodID(clzPathClassLoader, "<init>",
                                              "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");

    LOGI("PathClassLoader loading jarpath[%s]\n", jarpath);
    LOGI("nativepath loading nativepath[%s]\n", nativepath);

    jstring jarpath_str = env->NewStringUTF(jarpath);
    jstring narivepath_str = env->NewStringUTF(nativepath);

    jobject myClassLoader = env->NewObject(clzPathClassLoader, mdinitPathCL, jarpath_str, narivepath_str,
                                           NULL);
    env->DeleteLocalRef(narivepath_str);
    env->DeleteLocalRef(jarpath_str);
    return myClassLoader;
}

jclass findClassFromLoader(JNIEnv *env, jobject class_loader, const char *class_name) {
    jclass clz = env->GetObjectClass(class_loader);

    jmethodID mid = env->GetMethodID(clz, "loadClass",
                                     "(Ljava/lang/String;)Ljava/lang/Class;");
    jclass ret = nullptr;
    if (!mid) {
        mid = env->GetMethodID(clz, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    }
    jobject target = env->CallObjectMethod(class_loader, mid,
                                           env->NewStringUTF(class_name));
    if (target) {
        return (jclass) target;
    }

    LOGE("Class %s not found", class_name);

    return ret;
}