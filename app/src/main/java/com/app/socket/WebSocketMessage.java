package com.app.socket;

public class WebSocketMessage {
    //发送标识
    private String id;

    //发送的数据类型
    private Type type;

    //消息内容
    private String message;

    //字节数组数据
    private byte[] dataBuf;

    public static enum Type {
        TEXT,
        LAYOUT,
        IMG,
        GET_LAYOUT,
        GET_LAYOUT_IMG,
        GET_LAYOUT_IMG_END,
    }

    private WebSocketMessage() {
    }

    public static WebSocketMessage createTextMessage(String id, String message) {
        WebSocketMessage webSocketMessage = new WebSocketMessage();

        webSocketMessage.setId(id);
        webSocketMessage.setMessage(message);
        webSocketMessage.setType(Type.TEXT);

        return webSocketMessage;
    }

    public static WebSocketMessage createImgMessage(String id, byte[] dataBuf) {
        WebSocketMessage webSocketMessage = new WebSocketMessage();

        webSocketMessage.setId(id);
        webSocketMessage.setDataBuf(dataBuf);
        webSocketMessage.setType(Type.IMG);

        return webSocketMessage;
    }

    public static WebSocketMessage createLayoutMessage(String id, String message) {
        WebSocketMessage webSocketMessage = new WebSocketMessage();

        webSocketMessage.setId(id);
        webSocketMessage.setMessage(message);
        webSocketMessage.setType(Type.LAYOUT);
        return webSocketMessage;
    }

    public static WebSocketMessage createMessage(String id, Type type, String message, byte[] dataBuf) {
        WebSocketMessage webSocketMessage = new WebSocketMessage();

        webSocketMessage.setId(id);
        webSocketMessage.setDataBuf(dataBuf);
        webSocketMessage.setMessage(message);
        webSocketMessage.setType(type);

        return webSocketMessage;
    }

    public static WebSocketMessage createMessage(Type type) {
        return createMessage("0", type, "", null);
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public Type getType() {
        return type;
    }

    public void setType(Type type) {
        this.type = type;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public byte[] getDataBuf() {
        return dataBuf;
    }

    public void setDataBuf(byte[] dataBuf) {
        this.dataBuf = dataBuf;
    }
}
