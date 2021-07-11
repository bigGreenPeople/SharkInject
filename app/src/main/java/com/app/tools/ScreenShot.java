package com.app.tools;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.view.View;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class ScreenShot {
    public final static String TAG = "SharkChilli";

    // 获取指定Activity的截屏，保存到png文件
    private static Bitmap takeScreenShot(View view) {
        // View是你需要截图的View
        view.setDrawingCacheEnabled(true);
        view.buildDrawingCache();
        Bitmap b1 = view.getDrawingCache();

        // 获取屏幕长和高
        int width = view.getWidth();
        int height = view.getHeight();
        Bitmap b = Bitmap.createBitmap(b1, 0, 0, width, height);
        view.destroyDrawingCache();
        return b;
    }

    // 获取指定Activity的截屏，保存到png文件
    private static Bitmap takeScreenShot(Activity activity) {
        // View是你需要截图的View
        View view = activity.getWindow().getDecorView();
        view.setDrawingCacheEnabled(true);
        view.buildDrawingCache();
        Bitmap b1 = view.getDrawingCache();

        // 获取状态栏高度
        Rect frame = new Rect();
        activity.getWindow().getDecorView().getWindowVisibleDisplayFrame(frame);
        int statusBarHeight = frame.top;
//        Log.i(TAG, "" + statusBarHeight);

        // 获取屏幕长和高
        int width = activity.getWindowManager().getDefaultDisplay().getWidth();
        int height = activity.getWindowManager().getDefaultDisplay()
                .getHeight();
        // 去掉标题栏
        // Bitmap b = Bitmap.createBitmap(b1, 0, 25, 320, 455);
        Bitmap b = Bitmap.createBitmap(b1, 0, statusBarHeight, width, height
                - statusBarHeight);
        view.destroyDrawingCache();
        return b;
    }

    // 保存到sdcard
    private static void savePic(Bitmap b, String strFileName) {
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(strFileName);
            if (null != fos) {
                b.compress(Bitmap.CompressFormat.PNG, 90, fos);
                fos.flush();
                fos.close();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 获取截屏的byte数组
     *
     * @param activity
     * @return
     */
    public static byte[] getActivityScreenBytes(Activity activity) {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();

        Bitmap bitmap = ScreenShot.takeScreenShot(activity);
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, byteArrayOutputStream);
        byte[] bytes = byteArrayOutputStream.toByteArray();
        return bytes;
    }

    /**
     * 获取截屏的byte数组
     *
     * @param view
     * @return
     */
    public static byte[] getViewScreenBytes(View view) {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();

        Bitmap bitmap = ScreenShot.takeScreenShot(view);
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, byteArrayOutputStream);
        byte[] bytes = byteArrayOutputStream.toByteArray();
        return bytes;
    }

    // 程序入口
    public static void shoot(Activity a) {
        ScreenShot.savePic(ScreenShot.takeScreenShot(a), "sdcard/YoupinSeckillSuccess.png");
    }


    // 程序入口
    public static void shoot(View a) {
        ScreenShot.savePic(ScreenShot.takeScreenShot(a), "sdcard/YoupinSeckillSuccess.png");
    }

    // 程序入口
    public static void shoot(View a, String savePath) {
        ScreenShot.savePic(ScreenShot.takeScreenShot(a), savePath);
    }

    // 程序入口
    public static void shoot(Activity a, String savePath) {
        ScreenShot.savePic(ScreenShot.takeScreenShot(a), savePath);
    }
}
