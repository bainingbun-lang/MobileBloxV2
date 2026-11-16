package com.mobileblox.v2.launcher

import android.content.Context
import android.content.Intent

/**
 * Opens the installed Roblox client. No client modification is performed: the
 * package manager only resolves Roblox's own launcher activity.
 */
object RobloxLauncher {

    private val packages = listOf(
        "com.roblox.client",
        "com.roblox.client.beta",
    )

    fun isInstalled(context: Context): Boolean =
        packages.any { context.packageManager.getLaunchIntentForPackage(it) != null }

    fun launch(context: Context): Boolean {
        for (packageName in packages) {
            val intent = context.packageManager.getLaunchIntentForPackage(packageName) ?: continue
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            if (runCatching { context.startActivity(intent) }.isSuccess) {
                return true
            }
        }
        return false
    }
}
