package com.Plugins.SignInAndroidHelper;

import android.util.Log;
import androidx.annotation.Keep;

@Keep
public class SignInAndroidHelper
{
    public static String helloFromAndroid() 
    {
        Log.d("HelloBridge", "helloFromAndroid");
        return "HelloFromAndroid";
    }
}