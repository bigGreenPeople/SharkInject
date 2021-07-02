package com.app.socket;

import android.os.Handler;
import android.util.Log;

import com.app.context.ContextUtils;
import com.app.signal.IRecvListener;
import com.google.gson.Gson;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;

public class JWebSocketClient extends WebSocketClient {
    public final static String TAG = "SharkChilli";
    private static final long HEART_BEAT_RATE = 10 * 1000;//每隔10秒进行一次对长连接的心跳检测
    private Handler mHandler;

    private IRecvListener mIRecvListener;

    private Runnable heartBeatRunnable = new Runnable() {
        @Override
        public void run() {
            if (isClosed()) {
//                Log.i(TAG, "开启重连");
                reconnectWs();
            }
            //定时对长连接进行心跳检测
            mHandler.postDelayed(this, HEART_BEAT_RATE);
        }
    };

    /**
     * 开启重连
     */
    private void reconnectWs() {
        mHandler.removeCallbacks(heartBeatRunnable);
        new Thread() {
            @Override
            public void run() {
                try {
                    //重连
                    Log.i(TAG, "reconnectWs.................... ");
                    reconnect();
                } catch (Exception e) {
                    Log.e(TAG, "run: ", e);
                }
            }
        }.start();
    }

    public JWebSocketClient(URI serverUri, IRecvListener iRecvListener) {
        super(serverUri);
        ContextUtils contextUtils = ContextUtils.getInstance();
        this.mIRecvListener = iRecvListener;

        contextUtils.runOnUiThread(() -> {
            Log.i(TAG, "runOnUiThread: ");
            mHandler = new Handler();
            Log.i(TAG, "mHandler: " + mHandler);
        });
        try {
            Thread.sleep(3 * 1000);
        } catch (InterruptedException e) {
            Log.e(TAG, "JWebSocketClient: ", e);
        }
    }

    public void send(WebSocketMessage webSocketMessage) {
        String json = new Gson().toJson(webSocketMessage);
        Log.i(TAG, "send: " + json);
        this.send(json);
    }

    @Override
    public void onOpen(ServerHandshake serverHandshake) {
        Log.e(TAG, "onOpen()");
        mHandler.postDelayed(heartBeatRunnable, HEART_BEAT_RATE);//开启心跳检测
    }

    @Override
    public void onMessage(String message) {
        Log.e(TAG, "onMessage:" + message);
        mIRecvListener.recvMessage(message);
    }

    @Override
    public void onClose(int code, String reason, boolean remote) {
        Log.e(TAG, "onClose()");
    }

    public void sendMessage(String message) {
        if (!isClosed()) {
            this.send(message);
        }
    }

    public void closeConnect() {
        if (this.isOpen()) {
            this.close();
        }
    }

    @Override
    public void onError(Exception ex) {
        Log.e(TAG, "onError()", ex);
    }
}
