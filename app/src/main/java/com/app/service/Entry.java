package com.app.service;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.ArrayMap;
import android.util.Log;

import com.app.context.ContextUtils;
import com.app.logic.LogicEntry;

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
        LogicEntry logicEntry = new LogicEntry();
        //添加相应的变量
        logicEntry.onLoad(classLoader, pkgName, flag);
        //初始化相关hook
        logicEntry.init();
        //调用入口
        logicEntry.entry();
        //获取当前activity
        ContextUtils contextUtils = ContextUtils.getInstance(classLoader);
        Activity topActivity = contextUtils.getTopActivity();

        logicEntry.activityOnCreate(topActivity);
    }

}
