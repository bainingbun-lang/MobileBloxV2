package com.mobileblox.v2

import android.app.Application
import com.mobileblox.v2.bridge.BridgeService
import timber.log.Timber

class MobileBloxApplication : Application() {

    override fun onCreate() {
        super.onCreate()

        // Initialize Timber for logging
        if (BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        }

        // Native Luau module is loaded lazily by com.mobileblox.v2.luau.LuauBridge.

        // Local automation bridge (localhost only, token protected)
        BridgeService.start(this)
    }

    override fun onTerminate() {
        BridgeService.stop()
        super.onTerminate()
    }
}
