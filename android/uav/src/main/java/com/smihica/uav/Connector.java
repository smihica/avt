package com.smihica.uav;

import java.net.Socket;
import java.net.UnknownHostException;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.InputStreamReader;
import java.io.IOException;
import android.util.Log;

public class Connector extends Thread {

    public final String TAG = "com.smihica.bot.Connector";

    private Main main;
    private Socket connection = null;
    private BufferedReader reader = null;
    private PrintWriter writer = null;

    private final String TARGET_HOST = "192.168.108.25";
    private final int TARGET_PORT = 10002;
    private final int RETRY_TIMESPAN = 3000;

    private boolean shutdownRequired = false;

    public Connector(Main m) {
        this.main = m;
    }

    @Override
    public void run() {
    outer:
        while (true) {
            if (shutdownRequired) return;
            boolean error = this.connection_start();
            if (error) {
                try {
                    Thread.sleep(RETRY_TIMESPAN);
                } catch (InterruptedException e) {
                    main.resetConfigure();
                    return;
                }
                continue;
            }
            main.log("connection accepted");
            while (true) {
                // サーバーからのメッセージを受信
                if (shutdownRequired) return;
                try {
                    String message = reader.readLine();
                    if (message == null) {
                        main.resetConfigure();
                        throw new IOException("invalid message");
                    }
                    main.changeConfigure(message);
                } catch (IOException e) {
                    main.log("Error：" + e.toString());
                    connection_close();
                    main.resetConfigure();
                    continue outer;
                }
            }
        }
    }

    public void println(String str) {
        if (writer != null) {
            writer.println(str);
        }
    }

    public void shutdown() {
        shutdownRequired = true;
        connection_close();
    }

    public boolean connection_close() {
        try {
            if (connection != null) { connection.close(); connection = null; }
            if (reader != null) { reader.close(); reader = null; }
            if (writer != null) { writer.close(); writer = null; }
        } catch (Exception e) {
            e.printStackTrace();
            main.log("エラー内容：" + e.toString());
            main.log("サーバーとの接続に失敗しました。");
            return true;
        }
        return false;
    }

    private boolean connection_start() {
        try {
            connection = new Socket(TARGET_HOST, TARGET_PORT);
            writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(connection.getOutputStream())), true);
            reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            writer.println("BOT");
        } catch (UnknownHostException e) {
            e.printStackTrace();
            main.log("Error：" + e.toString());
            connection_close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            main.log("Error：" + e.toString());
            connection_close();
            return true;
        }
        return false;
    }
}
