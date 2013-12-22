package com.smihica.uav;

import jp.ksksue.driver.serial.*;

public class Conf {
    public static final String TAG = "com.smihica.bot.Conf";

    public static final int     SERIAL_BAUDRATE = FTDriver.BAUD19200;

    public static final String     LINEFEED_CODE_CR = "\r";
    public static final String     LINEFEED_CODE_CRLF = "\r\n";
    public static final String     LINEFEED_CODE_LF = "\n";
    public static final String     LINEFEED_CODE = LINEFEED_CODE_CR;

    public static final int     OUTPUT_TYPE_STRING = 0;
    public static final int     OUTPUT_TYPE_NUMBER = 1;
    public static final int     OUTPUT_TYPE_HEXSTR = 2;
    public static final int     OUTPUT_TYPE = OUTPUT_TYPE_STRING;

    public static final int     READ_TIMESPAN = 20; // ms

    public static final int     ACCEL_DEFAULT = 160;
    public static final int     DIRECTION_DEFAULT = 160;
    public static final int     PAN_DEFAULT = 162;
    public static final int     BATT_SW1_DEFAULT = 0;
    public static final int     BATT_SW2_DEFAULT = 0;
}
