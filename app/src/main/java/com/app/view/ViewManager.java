package com.app.view;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Rect;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.TextView;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ViewManager {
    private static ViewManager mViewHelper;
    private ClassLoader mClassLoader;

    private ViewManager() {
    }

    private ViewManager(ClassLoader classLoader) {
        this.mClassLoader = classLoader;
    }

    public static synchronized ViewManager getInstance(ClassLoader classLoader) {
        if (mViewHelper == null) {
            mViewHelper = new ViewManager(classLoader);
        }
        return mViewHelper;
    }

    @SuppressLint({"PrivateApi", "DiscouragedPrivateApi"})
    public ArrayList<View> getWindowsView(Activity activity) {
        try {
            Class WindowManagerGlobalClass = activity.getClassLoader().loadClass("android.view.WindowManagerGlobal");
            Method getInstance = WindowManagerGlobalClass.getDeclaredMethod("getInstance");
            getInstance.setAccessible(true);
            Object mGlobal = getInstance.invoke(null);

            Field mViewsField = WindowManagerGlobalClass.getDeclaredField("mViews");
            mViewsField.setAccessible(true);
            ArrayList<View> mViews = (ArrayList<View>) mViewsField.get(mGlobal);
            return mViews;
        } catch (Exception e) {
            Log.e("SharkChilli", "onCreate: ", e);
        }
        return null;
    }

    public Map<String, ViewInfo> getActivitysLayout(List<Activity> activities) {
        HashMap<String, ViewInfo> layoutMap = new HashMap<>();

        activities.forEach(activity -> {
//            layoutMap.put(activity.getClass().getName(), getActivityViewInfo(activity));
            // 添加每个activity 的windows
            ArrayList<ViewInfo> windowViewInfo = getWindowViewInfo(activity);

            for (int i = 0; i < windowViewInfo.size(); i++) {
                layoutMap.put(activity.getClass().getName() + i, windowViewInfo.get(i));
            }
        });

        return layoutMap;
    }


    public ViewInfo getActivityViewInfo(Activity activity) {
        Window window = activity.getWindow();
        View decorView = window.getDecorView();
        View viewById = decorView.findViewById(Window.ID_ANDROID_CONTENT);
        int statusBarHeight = getStatusBarHeight(activity);
        return getViewInfo(viewById, statusBarHeight);
    }

    public ArrayList<ViewInfo> getWindowViewInfo(Activity activity) {
        ArrayList<ViewInfo> viewInfos = new ArrayList<>();
        try {
            ArrayList<View> mViews = getWindowsView(activity);
            Log.i("SharkChilli", "mViews size is:" + mViews.size());
            int statusBarHeight = getStatusBarHeight(activity);

            for (int i = 0; i < mViews.size(); i++) {
                View viewById = mViews.get(i);
                if (i == 0){
                    viewById = viewById.findViewById(Window.ID_ANDROID_CONTENT);
                }
                viewInfos.add(getViewInfo(viewById, statusBarHeight));
            }
        } catch (Exception e) {
            Log.e("SharkChilli", " ", e);
        }
        return viewInfos;
    }

    public int getStatusBarHeight(Activity activity) {
        Resources resources = activity.getResources();
        int resourceId = resources.getIdentifier("status_bar_height", "dimen", "android");
        int height = resources.getDimensionPixelSize(resourceId);
        return height;
    }

    public ViewInfo getViewInfo(View view, int statusBarHeight) {
        ViewInfo viewInfo = new ViewInfo();

        viewInfo.setId(view.getId());
        viewInfo.setClassName(view.getClass().getName());
        viewInfo.setEnabled(view.isEnabled());
        viewInfo.setShown(view.isShown());
        viewInfo.setHeight(view.getHeight());
        viewInfo.setWidth(view.getWidth());
        int[] viewLocation = getViewLocation(view);
        viewInfo.setX(viewLocation[0]);
        viewInfo.setY(viewLocation[1] - statusBarHeight);
        if (view.getContentDescription() != null)
            viewInfo.setDescription(view.getContentDescription().toString());

        if (view instanceof TextView) {
            viewInfo.setText(((TextView) view).getText().toString());
//            Log.i("SharkChilli", "getViewInfo: " + ((TextView) view).getText().toString());
        }

        if (view instanceof ViewGroup) {
            ArrayList<ViewInfo> childList = new ArrayList<>();

            ViewGroup vp = (ViewGroup) view;

            for (int i = 0; i < vp.getChildCount(); i++) {
                View viewchild = vp.getChildAt(i);
                ViewInfo childViewInfo = getViewInfo(viewchild, statusBarHeight);
                childList.add(childViewInfo);
            }
            viewInfo.setChildList(childList);
        }

        return viewInfo;
    }

    /**
     * 获取控件位置
     *
     * @return
     */
    public int[] getViewLocation(View view) {
        int[] location = new int[2];
//        view.getLocationInWindow(location);

//        Log.i("SharkChilli", "getLocationInWindow: " + location[0] +":"+location[1]);
        view.getLocationOnScreen(location);
//        Log.i("SharkChilli", "getLocationOnScreen: " + location[0] +":"+location[1]);

        return location;
    }


    private boolean isViewVisible(View host, Rect window, boolean fullVisible) {
        // Make sure our host view is attached to a visible window.
        if (host.getWindowVisibility() == View.VISIBLE) {
            // An invisible predecessor or one with alpha zero means
            // that this view is not visible to the user.
            Object current = host;
            while (current instanceof View) {
                View view = (View) current;
                // We have attach info so this view is attached and there is no
                // need to check whether we reach to ViewRootImpl on the way up.
                if (view.getAlpha() <= 0 || view.getVisibility() != View.VISIBLE) {
                    return false;
                }
                current = view.getParent();
            }

            // Check if the view is visible in window.
            // host.getWindowVisibleDisplayFrame(mWindowRect);
            Rect visibleRect = new Rect();
            if (fullVisible) {
                // Check if the view is entirely visible.
                if (!host.getLocalVisibleRect(visibleRect)) {
                    return false;
                }

                return visibleRect.top == 0 && visibleRect.left == 0 &&
                        visibleRect.bottom == host.getHeight() && visibleRect.right == host.getWidth();
            } else {
                // Check if the view is entirely covered by its predecessors.
                if (!host.getGlobalVisibleRect(visibleRect)) {
                    return false;
                }
                return Rect.intersects(window, visibleRect);
            }
        }
        return false;
    }
}