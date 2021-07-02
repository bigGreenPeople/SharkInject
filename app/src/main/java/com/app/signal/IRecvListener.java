package com.app.signal;

/**
 * 通信消息监听者
 */
public interface IRecvListener {
    /**
     * 接收消息
     * @param message
     */
    void recvMessage(String message);
}