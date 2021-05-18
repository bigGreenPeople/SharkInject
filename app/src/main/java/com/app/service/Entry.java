package com.app.service;

import android.util.ArrayMap;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;

import dalvik.system.DexClassLoader;
import dalvik.system.PathClassLoader;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;

public class Entry {
    public static String TAG = "SharkChilli";

    public static void onLoad(PathClassLoader classLoader, String pkgName, boolean flag) {
//        ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
//        ClassLoader systemClassLoader = null;
//        try {
//            systemClassLoader = getCurClassloder();
//        } catch (Exception e) {
//            Log.e(TAG, "onLoad: ", e);
//        }
//        Log.i(TAG, "onLoad systemClassLoader: " + systemClassLoader.toString());
//        ClassLoader parent = systemClassLoader.getParent();
//        while (parent != null) {
//            Log.i(TAG, "parent: " + parent.toString());
//            parent = parent.getParent();
//        }
//        Log.i(TAG, "onLoad pkgName: " + pkgName);
//        Log.i(TAG, "onLoad classLoader: " + classLoader.toString());
//        parent = classLoader.getParent();
//        while (parent != null) {
//            Log.i(TAG, "parent: " + parent.toString());
//            parent = parent.getParent();
//        }

        XposedBridge.log("Shark ok!!!!");
//        classLoader.findLibrary()
        /*try {
            Class SharkUitlsClass = classLoader.loadClass("com.shark.nougat.SharkUitls");
            XposedHelpers.findAndHookMethod(SharkUitlsClass, "add", int.class, int.class, new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    super.beforeHookedMethod(param);
                }

                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    super.afterHookedMethod(param);
                    param.setResult(666);
                }
            });
        } catch (Exception e) {
            Log.e(TAG, "onLoad: ", e);
        }*/
    }

    public static ClassLoader getCurClassloder() throws Exception {
        Class ApplicationLoadersClass = Class.forName("android.app.ApplicationLoaders");
        Field gApplicationLoadersField = ApplicationLoadersClass.getDeclaredField("gApplicationLoaders");
        gApplicationLoadersField.setAccessible(true);
        Object gApplicationLoaders = gApplicationLoadersField.get(null);

        Field mLoadersField = ApplicationLoadersClass.getDeclaredField("mLoaders");
        mLoadersField.setAccessible(true);
        ArrayMap<String, Object> mLoaders = (ArrayMap) mLoadersField.get(gApplicationLoaders);

        for (Map.Entry<String, Object> entry : mLoaders.entrySet()) {
//            Log.i(TAG, "entry key: " + entry.getKey());
//            Log.i(TAG, "entry val: " + entry.getValue());
            if (entry.getValue() == null) return null;
            return (ClassLoader) entry.getValue();
        }

        return null;
    }
}
