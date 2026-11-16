package com.mobileblox.v2

import android.app.Application
import android.util.Log
import timber.log.Timber

class MobileBloxApplication : Application() {

    override fun onCreate() {
        super.onCreate()

        // Initialize Timber for logging
        if (BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        }

        // Load native library
        try {
            System.loadLibrary("MobileBlox")
            Log.i("MobileBlox", "Native library loaded successfully")
        } catch (e: UnsatisfiedLinkError) {
            Log.e("MobileBlox", "Failed to load native library", e)
        }
    }
}