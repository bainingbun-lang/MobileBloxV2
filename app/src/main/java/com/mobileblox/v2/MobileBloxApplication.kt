package com.mobileblox.v2

import android.app.Application
import android.util.Log
import com.mobileblox.v2.bridge.BridgeService
import timber.log.Timber

class MobileBloxApplication : Application() {

    override fun onCreate() {
        super.onCreate()

        // Initialize Timber for logging
        if (BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        }

        // Load native library (optional until the Luau/native module is built)
        try {
            System.loadLibrary("MobileBlox")
            Log.i("MobileBlox", "Native library loaded successfully")
        } catch (e: UnsatisfiedLinkError) {
            Log.e("MobileBlox", "Failed to load native library", e)
        }

        // Local automation bridge (localhost only, token protected)
        BridgeService.start(this)
    }

    override fun onTerminate() {
        BridgeService.stop()
        super.onTerminate()
    }
}
