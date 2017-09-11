package com.sxy.daemonprocessdemo;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.util.Log;

import java.util.Timer;
import java.util.TimerTask;

/**
 *
 * Created by sunxiaoyu on 2017/9/7.
 */

public class ProcessSocketService extends Service {

    private static final String TAG = "sunxiaoyu";
    int i=0;

    @Override
    public void onCreate() {
        super.onCreate();

        createWatcher((Process.myUid()));
        connectToMonitor();

        Timer timer = new Timer();
        //定时器
        timer.scheduleAtFixedRate(
                new TimerTask() {
                    public void run() {
                        Log.i(TAG, "ProcessService在运行：  " + i++);
                    }
                }, 0, 1000 * 3);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public native void createWatcher(int userId);
    public native void connectToMonitor();

    static {
        System.loadLibrary("daemonProcess-lib");
    }
}
