package com.app.logic;

import android.app.Activity;
import android.util.Log;

import com.app.service.AbstractEntry;

public class LogicEntry extends AbstractEntry {
    @Override
    public void activityOnCreate(Activity activity) {
        Log.i(TAG, " activityOnCreate: " + activity.getClass().getName());
    }

    @Override
    public void entry() {

    }
}
