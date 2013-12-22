package com.smihica.uav;

import jp.ksksue.driver.serial.*;

public class DeviceReader extends Thread {

    public final String TAG = "com.smihica.bot.DeviceReader";

    private Main main;
    private FTDriver serial;
    private boolean shutdownRequired = false;

    public DeviceReader(Main m, FTDriver s) {
        this.main = m;
        this.serial = s;
    }

    private String stringify(byte[] rbuf, int len) {
        if (len <= 0) return "";
        String mText = "";
        for(int i=0;i<len;++i) {
            switch(Conf.OUTPUT_TYPE) {
            case Conf.OUTPUT_TYPE_STRING:
                // "\r":CR(0x0D) "\n":LF(0x0A)
                if (rbuf[i] == 0x0D) {
                    mText = mText + "\r";
                } else if (rbuf[i] == 0x0A) {
                    mText = mText + "\n";
                } else {
                    mText = mText + "" +(char)rbuf[i];
                }
                break;
            case Conf.OUTPUT_TYPE_NUMBER:
                if (rbuf[i] == 0x0D) {
                    mText = mText + " " + Byte.toString(rbuf[i]) + "\r";
                } else if (rbuf[i] == 0x0A) {
                    mText = mText + " " + Byte.toString(rbuf[i]) + "\n";
                } else {
                    mText = mText + " " + Byte.toString(rbuf[i]);
                }
                break;
            case Conf.OUTPUT_TYPE_HEXSTR:
                if (rbuf[i] == 0x0D) {
                    // TODO: output 2 length character (now not "0D", it's only "D".)
                    mText = mText + " " + Integer.toHexString((int) rbuf[i]) + "\r";
                } else if (rbuf[i] == 0x0A) {
                    mText = mText + " " + Integer.toHexString((int) rbuf[i]) + "\n";
                } else {
                    mText = mText + " "
                        + Integer.toHexString((int) rbuf[i]);
                }
                break;
            }
        }
        return mText;
    }

    public void shutdown() {
        shutdownRequired = true;
    }

    @Override
    public void run() {
        int i;
        int len;
        byte[] rbuf = new byte[4096];

        for(;;){//this is the main loop for transferring

            if (shutdownRequired) return;

            //////////////////////////////////////////////////////////
            // Read and Display to Terminal
            //////////////////////////////////////////////////////////

            if (!serial.isConnected()) return;
            len = serial.read(rbuf);
            final String res = stringify(rbuf, len);
            if (!res.equals("")) {
                if (res.charAt(0) == 'p') {
                    main.log(res);
                } else if (res.charAt(0) == 'b') {
                    main.onBatteryStatusChanged(res);
                }
            }

            if (0 < Conf.READ_TIMESPAN) {
                try {
                    Thread.sleep(Conf.READ_TIMESPAN);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    return;
                }
            }
        }
    }
}
