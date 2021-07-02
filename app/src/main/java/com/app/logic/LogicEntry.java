package com.app.logic;

import android.app.Activity;
import android.util.Log;
import android.widget.Toast;

import com.app.context.ContextUtils;
import com.app.service.AbstractEntry;
import com.app.signal.IRecvListener;
import com.app.socket.JWebSocketClient;
import com.app.socket.WebSocketMessage;
import com.app.tools.ScreenShot;
import com.app.view.ViewInfo;
import com.app.view.ViewManager;
import com.google.gson.Gson;

import java.net.URI;
import java.util.Map;

public class LogicEntry extends AbstractEntry implements IRecvListener {
    private static JWebSocketClient mJWebSocketClient;
    private ContextUtils mContextUtils;
    private ViewManager mViewManager;

    @Override
    public void activityOnCreate(Activity activity) {
        Log.i(TAG, " activityOnCreate: " + activity.getClass().getName());
    }

    @Override
    public void entry() {
        Log.i(TAG, "entry: ");
        mContextUtils = ContextUtils.getInstance(this.mClassLoader);
        mViewManager = ViewManager.getInstance(this.mClassLoader);

        URI uri = URI.create("ws://192.168.124.7:9873");
        mJWebSocketClient = new JWebSocketClient(uri, this);
        if (mJWebSocketClient != null) {
            try {
                mJWebSocketClient.connectBlocking();
            } catch (Exception e) {
                Log.e(TAG, "onLoad: ", e);
            }
        }

        if (mJWebSocketClient.isOpen()) {
            Log.i(TAG, "连接成功");
        } else
            Log.i(TAG, "连接失败!!!");
    }

    @Override
    public void recvMessage(String message) {
        WebSocketMessage webSocketMessage = new Gson().fromJson(message, WebSocketMessage.class);
        if (WebSocketMessage.Type.GET_LAYOUT_IMG.equals(webSocketMessage.getType())) {
            if (mJWebSocketClient == null) {
                Log.i(TAG, "mJWebSocketClient == null");
                return;
            }

            mContextUtils.getRunningActivitys().forEach(activity -> {
                byte[] activityScreenBytes = ScreenShot.getActivityScreenBytes(activity);
                mJWebSocketClient.send(activityScreenBytes);
            });
            // 发送完毕
            WebSocketMessage textMessage = WebSocketMessage.createMessage(WebSocketMessage.Type.GET_LAYOUT_IMG_END);
            mJWebSocketClient.send(textMessage);
        } else if (WebSocketMessage.Type.GET_LAYOUT.equals(webSocketMessage.getType())) {
            Map<String, ViewInfo> activitysLayout = mViewManager.getActivitysLayout(mContextUtils.getRunningActivitys());
            String activitysLayoutInfo = new Gson().toJson(activitysLayout);
            WebSocketMessage textMessage = WebSocketMessage.createLayoutMessage("0", activitysLayoutInfo);
            mJWebSocketClient.send(textMessage);
        }
    }
}
