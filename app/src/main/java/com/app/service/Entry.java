package com.app.service;

import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import dalvik.system.PathClassLoader;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;

public class Entry {
    public static String TAG = "SharkChilli";

    public static void onLoad(PathClassLoader classLoader, String pkgName, boolean flag) {
//        ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
//        Log.i(TAG, "onLoad systemClassLoader: " + systemClassLoader.toString());
        Log.i(TAG, "onLoad pkgName: " + pkgName);
//        Log.i(TAG, "onLoad classLoader: " + classLoader.toString());
//        classLoader.findLibrary()
        try {
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
        }
    }
}
