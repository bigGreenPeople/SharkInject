package com.app.view;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.TextView;

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

    public Map<String, ViewInfo> getActivitysLayout(List<Activity> activities) {
        HashMap<String, ViewInfo> layoutMap = new HashMap<>();

        activities.forEach(activity -> {
            layoutMap.put(activity.getClass().getName(), getActivityViewInfo(activity));
        });

        return layoutMap;
    }

    public ViewInfo getActivityViewInfo(Activity activity) {
        Window window = activity.getWindow();
        View decorView = window.getDecorView();
        return getViewInfo(decorView);
    }

    public ViewInfo getViewInfo(View view) {
        ViewInfo viewInfo = new ViewInfo();

        viewInfo.setId(view.getId());
        viewInfo.setClassName(view.getClass().getName());
        viewInfo.setEnabled(view.isEnabled());
        viewInfo.setShown(view.isShown());
        viewInfo.setHeight(view.getHeight());
        viewInfo.setWidth(view.getWidth());
        int[] viewLocation = getViewLocation(view);
        viewInfo.setX(viewLocation[0]);
        viewInfo.setY(viewLocation[1]);
        if (view.getContentDescription() != null)
            viewInfo.setDescription(view.getContentDescription().toString());

        if (view instanceof TextView)
            viewInfo.setText(((TextView) view).getText().toString());

        if (view instanceof ViewGroup) {
            ArrayList<ViewInfo> childList = new ArrayList<>();

            ViewGroup vp = (ViewGroup) view;

            for (int i = 0; i < vp.getChildCount(); i++) {
                View viewchild = vp.getChildAt(i);
                ViewInfo childViewInfo = getViewInfo(viewchild);
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
        view.getLocationOnScreen(location);
        return location;
    }

}
