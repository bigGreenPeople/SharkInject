package com.app.service;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import dalvik.system.PathClassLoader;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;

public abstract class AbstractEntry {
    protected static String TAG = "SharkChilli";
    public PathClassLoader mClassLoader;
    private String pkgName;

    public AbstractEntry() {
//        init();
    }

    public abstract void activityOnCreate(Activity activity);

    public void init() {
        try {
            Class ActivityClass = mClassLoader.loadClass("android.app.Activity");

            XposedHelpers.findAndHookMethod(ActivityClass, "onCreate", Bundle.class,
                    new XC_MethodHook() {
                        @Override
                        protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                            super.afterHookedMethod(param);
                            Activity activity = (Activity) param.thisObject;
                            activityOnCreate(activity);
                        }
                    });
        } catch (ClassNotFoundException e) {
            Log.i(TAG, "init: ", e);
        }
    }

    public void onLoad(PathClassLoader classLoader, String pkgName, boolean flag) {
        this.mClassLoader = classLoader;
    }

    public abstract void entry();
}
