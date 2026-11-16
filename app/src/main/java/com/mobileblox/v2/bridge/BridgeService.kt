package com.mobileblox.v2.bridge

import android.content.Context
import android.os.Build
import com.mobileblox.v2.BuildConfig
import fi.iki.elonen.NanoHTTPD
import timber.log.Timber
import java.io.File

object BridgeService {
    const val PORT = 9611

    val scriptQueue = ScriptQueue()

    @Volatile
    var token: String = ""
        private set

    @Volatile
    var running: Boolean = false
        private set

    @Volatile
    var lastError: String? = null
        private set

    private var server: BridgeServer? = null

    fun start(context: Context) {
        if (server != null) return
        token = BridgeTokenStore.getOrCreate(context)
        val files = FileRepository(File(context.filesDir, "MobileBlox"))
        val dispatcher = BridgeDispatcher(files, HttpProxy(), scriptQueue) {
            mapOf(
                "name" to "MobileBloxV2",
                "version" to BuildConfig.VERSION_NAME,
                "platform" to "android",
                "api" to Build.VERSION.SDK_INT,
                "device" to Build.MODEL,
            )
        }
        val candidate = BridgeServer(PORT, dispatcher, token)
        try {
            candidate.start(NanoHTTPD.SOCKET_READ_TIMEOUT, false)
            server = candidate
            running = true
            lastError = null
            Timber.i("Bridge listening on 127.0.0.1:%d", PORT)
        } catch (e: Exception) {
            running = false
            lastError = e.message
            Timber.e(e, "Bridge failed to start")
        }
    }

    fun stop() {
        server?.stop()
        server = null
        running = false
    }
}
