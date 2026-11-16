package com.mobileblox.v2.bridge

import android.content.Context
import java.security.SecureRandom

object BridgeTokenStore {
    private const val PREFS = "mobileblox_bridge"
    private const val KEY = "token"

    fun getOrCreate(context: Context): String {
        val prefs = context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
        prefs.getString(KEY, null)?.let { return it }
        val bytes = ByteArray(16)
        SecureRandom().nextBytes(bytes)
        val token = bytes.joinToString("") { "%02x".format(it) }
        prefs.edit().putString(KEY, token).apply()
        return token
    }
}
