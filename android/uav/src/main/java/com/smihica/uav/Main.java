package com.smihica.uav;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.text.Editable;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import jp.ksksue.driver.serial.*;

public class Main extends Activity {

    public final String TAG = "com.smihica.bot.Main";

    private FTDriver serial;

    public int direction = 127;
    private SeekBar v_direction;
    public int camera_pan_h = 127;
    private SeekBar v_camera_pan_h;
    public int camera_pan_v = 127;
    private SeekBar v_camera_pan_v;
    public int accel = 127;
    private SeekBar v_accel;
    public int batt_sw1 = 0;
    public int batt_sw2 = 0;

    public String batt_status1 = "0000";
    public String batt_status2 = "0000";

    public Handler mHandler = new Handler();

    private TextView console;
    private ScrollView console_scroll;

    private DeviceReader    device_reader;
    private Connector       connector;
    private Streamer        streamer;
    private boolean         streaming = false;

    public void log(final String s) {
        mHandler.post(new Runnable() {
            public void run() {
            while (console.getLayout() != null &&
                   21 < console.getLayout().getLineCount()) {
                Editable text = console.getEditableText();
                int lineStart = console.getLayout().getLineStart(0);
                int lineEnd = console.getLayout().getLineEnd(0);
                text.delete(lineStart, lineEnd);
            }
            console.append(s+"\n");
            console_scroll.fullScroll(View.FOCUS_DOWN);
            }
        });
    }

    public void resetConfigure() {
        changeConfigure("w 127 127 127 127 0 0");
    }

    public void voiceOutput(String src) {
        log(src);
        int len = src.length();
        int start = Math.min(len, 2);
        int end   = Math.min(len, 92);
        writeDataToSerial("v " + src.substring(start, end) + "\r\n");
    }

    public void changeConfigure(String src) {
        if (src == null) return;
        log("received: " + src);
        char f = src.charAt(0);
        if (f == 'r') { resetConfigure(); return; }
        if (f == 'v') { voiceOutput(src); return; }
        Pattern pattern = Pattern.compile("^w (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)$");
        Matcher matcher = pattern.matcher(src);
        if (matcher.find()) {
            final String as = matcher.group(1);
            final String ds = matcher.group(2);
            final String hs = matcher.group(3);
            final String vs = matcher.group(4);
            final String b1 = matcher.group(5);
            final String b2 = matcher.group(6);
            mHandler.post(new Runnable() {
                public void run() {
                    accel = Integer.parseInt(as);
                    direction = Integer.parseInt(ds);
                    camera_pan_h = Integer.parseInt(hs);
                    camera_pan_v = Integer.parseInt(vs);
                    batt_sw1 = Integer.parseInt(b1);
                    batt_sw2 = Integer.parseInt(b2);
                    v_accel.setProgress(accel);
                    v_direction.setProgress(direction);
                    v_camera_pan_h.setProgress(camera_pan_h);
                    v_camera_pan_v.setProgress(camera_pan_v);
                    onConfigureChanged();
                }
            });
        } else {
            log("invalid format: " + src);
        }
    }

    public void onBatteryStatusChanged(String status) {
        Pattern pattern = Pattern.compile("^b (\\d\\d\\d\\d)(\\d\\d\\d\\d)$");
        Matcher matcher = pattern.matcher(status);
        if (matcher.find()) {
            final String bs1 = matcher.group(1);
            final String bs2 = matcher.group(2);
            log("battery: " + status);
            mHandler.post(new Runnable() {
                public void run() {
                    if (!(batt_status1.equals(bs1) && batt_status2.equals(bs2))) {
                    batt_status1 = bs1;
                    batt_status2 = bs2;
                    if (connector != null &&
                        connector.isAlive()) {
                        connector.println("b " + batt_status1 + batt_status2);
                        log("b " + batt_status1 + batt_status2);
                    }
                }
            }
        });
        } else {
            log("invalid format");
        }
    }

    public void onConfigureChanged () {
        log("configure changed: ACCEL: " + accel + " DIRECTION: " + direction + " PAN(H): " + camera_pan_h + " PAN(V): " + camera_pan_v + " BATT_SW1: " + batt_sw1 + " BATT_SW2: " + batt_sw2);
        writeDataToSerial("w " + accel + " " + direction + " " + camera_pan_h + " " + camera_pan_v + " " + batt_sw1 + " " + batt_sw2);
    }

    private void initUI() {
        console = (TextView)findViewById(R.id.console);
        console_scroll = (ScrollView)findViewById(R.id.console_scroll);

        final TextView direction_text = (TextView) findViewById(R.id.textDirection);
        v_direction = (SeekBar) findViewById(R.id.direction);
        v_direction.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar b) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                direction_text.setText("Direction:"+progress);
                direction = progress;
                if (fromUser) onConfigureChanged();
            }
        });
        v_direction.setMax(255);

        final TextView camera_pan_h_text = (TextView) findViewById(R.id.textCameraPanH);
        v_camera_pan_h = (SeekBar) findViewById(R.id.cameraPanH);
        v_camera_pan_h.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar b) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                camera_pan_h_text.setText("Camera pan (h):"+progress);
                camera_pan_h = progress;
                if (fromUser) onConfigureChanged();
            }
        });
        v_camera_pan_h.setMax(255);

        final TextView camera_pan_v_text = (TextView) findViewById(R.id.textCameraPanV);
        v_camera_pan_v = (SeekBar) findViewById(R.id.cameraPanV);
        v_camera_pan_v.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar b) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                camera_pan_v_text.setText("Camera pan (v):"+progress);
                camera_pan_v = progress;
                if (fromUser) onConfigureChanged();
            }
        });
        v_camera_pan_v.setMax(255);

        final TextView accel_text = (TextView) findViewById(R.id.textAccel);
        v_accel = (SeekBar) findViewById(R.id.accel);
        v_accel.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar b) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                accel_text.setText("Accel:"+progress);
                accel = progress;
                if (fromUser) onConfigureChanged();
            }
        });
        v_accel.setMax(255);

        final Button streaming_button = (Button) findViewById(R.id.streamingButton);
        streaming_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //if (!streaming) streamer.start(8081);
                // else streamer.stop();
                // streaming = !streaming;
            }
        });
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.bot);

        Window w = getWindow();
        if (w != null) w.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        streamer = new Streamer();
        streamer.open(8081);

        initUI();

        // ---------------------------------------------------------------------------------------
        // init receivers
        // ---------------------------------------------------------------------------------------

        // listen for new devices
        IntentFilter filter = new IntentFilter();
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);

        // for requesting permission
        // setPermissionIntent() before begin()
        filter.addAction(ACTION_USB_PERMISSION);

        registerReceiver(mUsbReceiver, filter);


        // get service
        serial = new FTDriver((UsbManager)getSystemService(Context.USB_SERVICE));
        PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
        serial.setPermissionIntent(permissionIntent);

    }

    private void writeDataToSerial(String strWrite) {
        if (serial != null && serial.isConnected()) {
            strWrite = changeLinefeedcode(strWrite);
            serial.write(strWrite.getBytes(), strWrite.length());
        } else {
            log("not attached yet.");
        }
    }

    private String changeLinefeedcode(String str) {
        str = str.replace("\\r", "\r");
        str = str.replace("\\n", "\n");
        str += str + Conf.LINEFEED_CODE;
        return str;
    }

    @Override
    public void onResume() {
        super.onResume();

        runConnector();

        Intent intent = getIntent();
        String action = intent.getAction();
        if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
            log("USB device attached ...");
            serial.usbAttached(intent);
            serial.begin(Conf.SERIAL_BAUDRATE);
        }

        runReader();
        resetConfigure();
    }

    @Override
    public void onPause() {
        super.onPause();
        if (device_reader != null) {
            if (device_reader.isAlive()) {
                device_reader.shutdown();
            }
            device_reader = null;
        }
        if (connector != null) {
            if (connector.isAlive()) {
                connector.shutdown();
            }
            connector = null;
        }
        if (serial != null) {
            serial.end();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (streamer != null) streamer.close();
        Window w = getWindow();
        if (w != null) w.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if (serial != null) {
            serial.end();
            serial = null;
        }
        unregisterReceiver(mUsbReceiver);
    }

    private void runConnector() {
        if (connector != null) {
            log("connector is already running.");
            return;
        }
        connector = new Connector(this);
        connector.start();
    }

    private void runReader() {
        if (!serial.isConnected()) {
            log("serial is not connected yet.");
            return;
        }
        if (device_reader != null) {
            log("device reader is already running.");
            return;
        }
        device_reader = new DeviceReader(this, serial);
        device_reader.start();
    }

    // BroadcastReceiver when insert/remove the device USB plug into/from a USB port
    BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                Main.this.log("attached");
                serial.usbAttached(intent);
                serial.begin(Conf.SERIAL_BAUDRATE);
                runReader();

            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                Main.this.log("detached");
                serial.usbDetached(intent);
                serial.end();
            } else if (ACTION_USB_PERMISSION.equals(action)) {
                log("Request permission");
                synchronized (this) {
                    if (!serial.isConnected()) {
                        log("Request permission begin");
                        serial.begin(Conf.SERIAL_BAUDRATE);
                    }
                }
                runReader();
            }
        }
    };

    private static final String ACTION_USB_PERMISSION = "com.smihica.bot.USB_PERMISSION";
}
