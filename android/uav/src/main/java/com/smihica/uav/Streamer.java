package com.smihica.uav;

public class Streamer {

    static {
        System.loadLibrary("streamer");
    }

    private native void init(int port);
    private native void exit();

    private boolean initialized = false;

    public Streamer() {}

    public void open(int port) {
        if (!initialized) { init(port); initialized = true; }
    }

    public void close() {
        if (initialized) exit();
        initialized = false;
    }
}
