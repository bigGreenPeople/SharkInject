package com.app.service;

import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import dalvik.system.PathClassLoader;

public class Entry {
    public static String TAG = "SharkChilli";

    public static void onLoad(PathClassLoader classLoader, String pkgName, boolean flag) {
        ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
        Log.i(TAG, "onLoad systemClassLoader: " + systemClassLoader.toString());
        Log.i(TAG, "onLoad pkgName: " + pkgName);
        Log.i(TAG, "onLoad classLoader: " + classLoader.toString());

        //使用app中的类
        try {
            Class SharkUitlsClass = classLoader.loadClass("com.shark.nougat.SharkUitls");
            Method add = SharkUitlsClass.getDeclaredMethod("add", int.class, int.class);
            int invoke = (int) add.invoke(SharkUitlsClass, 3, 2);
            Log.i(TAG, "invoke: "+invoke);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "ClassNotFoundException SharkUitls" );
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "NoSuchMethodException add" );
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }
}
