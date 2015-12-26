package com.tutk.sample.AVAPI;

import android.app.Activity;
import android.os.Bundle;
import com.tutk.sample.AVAPI.Client;
import com.tutk.sample.AVAPI.*;

public class MainActivity extends Activity
{
    static final String UID = "ZSCYLVKMKPYERAA9111A";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        (new Thread() {
            public void run() {
                Client.start(MainActivity.this.UID);
            }
        }).start();
    }
}
