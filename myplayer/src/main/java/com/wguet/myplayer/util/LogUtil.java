package com.wguet.myplayer.util;

import android.util.Log;

public class LogUtil {

    public static void d(String msg) {
        Log.d("LogUtil", msg);
    }

    public static void e(String msg) {
        Log.e("LogUtil", msg);
    }
}