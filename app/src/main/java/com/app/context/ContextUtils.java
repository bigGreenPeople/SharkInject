package com.app.context;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Application;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class ContextUtils {
    private static ContextUtils mContextUtils;
    private ClassLoader mClassLoader;

    private ContextUtils() {
    }

    private ContextUtils(ClassLoader classLoader) {
        this.mClassLoader = classLoader;
    }

    public static synchronized ContextUtils getInstance(ClassLoader classLoader) {
        if (mContextUtils == null) {
            mContextUtils = new ContextUtils(classLoader);
        }
        return mContextUtils;
    }

    public Application geApplication() {
        try {
            Application application =
                    (Application) mClassLoader.loadClass("android.app.ActivityThread")
                            .getMethod("currentApplication")
                            .invoke(null, (Object[]) null);
            return application;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public PackageManager getPackageManager() {
        PackageManager packageManager = geApplication().getPackageManager();
        return packageManager;
    }

    public List<Class> getAllActivityClass() {
        PackageManager packageManager = getPackageManager();
        ArrayList<Class> list = new ArrayList<>();

        Context context = geApplication();
        try {
            PackageInfo packageInfo = packageManager.getPackageInfo(context.getPackageName(), PackageManager.GET_ACTIVITIES);

            for (ActivityInfo activityInfo : packageInfo.activities) {
                Class aClass = Class.forName(activityInfo.name);
                list.add(aClass);
            }
        } catch (Exception exception) {
            throw new RuntimeException(exception);
        }
        return list;
    }

    public String getTopActivityName() {
        Application application = geApplication();
        ActivityManager activityManager = (ActivityManager) application.getSystemService(Application.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> runningTasks = activityManager.getRunningTasks(1);
        ActivityManager.RunningTaskInfo runningTaskInfo = runningTasks.get(0);
        return runningTaskInfo.topActivity.getClassName();
    }

    public Activity getTopActivity() {
        List<Activity> runningActivitys = getRunningActivitys();
        String topActivityName = getTopActivityName();
//        Log.i("SharkChilli", "getTopActivity: " + topActivityName);

        if (runningActivitys.size() == 1) {
            return runningActivitys.get(0);
        }

        for (Activity activityRun : runningActivitys) {
            if (topActivityName.equals(activityRun.getClass().getName())) {
                return activityRun;
            }
//            Log.i("SharkChilli", "activityRun: " + activityRun);
        }
        return null;
    }

    public List<Activity> getRunningActivitys() {
        List<Activity> list = new ArrayList<>();
        try {
            Class<Application> applicationClass = Application.class;
            Field mLoadedApkField = applicationClass.getDeclaredField("mLoadedApk");
            mLoadedApkField.setAccessible(true);

            Application application = geApplication();
            Object mLoadedApk = mLoadedApkField.get(application);
            Class<?> mLoadedApkClass = mLoadedApk.getClass();
            Field mActivityThreadField = mLoadedApkClass.getDeclaredField("mActivityThread");

            mActivityThreadField.setAccessible(true);
            Object mActivityThread = mActivityThreadField.get(mLoadedApk);
            Class<?> mActivityThreadClass = mActivityThread.getClass();
            Field mActivitiesField = mActivityThreadClass.getDeclaredField("mActivities");
            mActivitiesField.setAccessible(true);
            // ActivityThread.ActivityClientRecord
            Object mActivities = mActivitiesField.get(mActivityThread);
            // 注意这里一定写成Map，低版本这里用的是HashMap，高版本用的是ArrayMap
            if (mActivities instanceof Map) {
                @SuppressWarnings("unchecked")
                Map<Object, Object> arrayMap = (Map<Object, Object>) mActivities;
                for (Map.Entry<Object, Object> entry : arrayMap.entrySet()) {
                    Object value = entry.getValue();
                    Class<?> activityClientRecordClass = value.getClass();
                    Field activityField = activityClientRecordClass.getDeclaredField("activity");
                    activityField.setAccessible(true);
                    Object o = activityField.get(value);
                    list.add((Activity) o);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            list = null;
        }

        return list;
    }
}
