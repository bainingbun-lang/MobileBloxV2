package com.mobileblox.v2.shizuku

import android.content.pm.PackageManager
import android.os.ParcelFileDescriptor
import com.mobileblox.v2.BuildConfig
import moe.shizuku.server.IShizukuService
import rikka.shizuku.Shizuku

/**
 * Thin wrapper around Shizuku. Shizuku runs commands with the ADB shell UID,
 * which is enough to grant app permissions and app-ops, but it does NOT grant
 * access to the private memory of other applications.
 */
object ShizukuManager {

    const val REQUEST_CODE = 0x5348

    data class ShellResult(
        val code: Int,
        val stdout: String,
        val stderr: String,
    ) {
        val ok: Boolean get() = code == 0
    }

    private val grantablePermissions = setOf(
        "android.permission.POST_NOTIFICATIONS",
        "android.permission.READ_EXTERNAL_STORAGE",
        "android.permission.WRITE_EXTERNAL_STORAGE",
        "android.permission.READ_MEDIA_IMAGES",
        "android.permission.READ_MEDIA_VIDEO",
        "android.permission.READ_MEDIA_AUDIO",
    )

    fun isAvailable(): Boolean = try {
        Shizuku.pingBinder()
    } catch (t: Throwable) {
        false
    }

    fun hasPermission(): Boolean = try {
        isAvailable() && Shizuku.checkSelfPermission() == PackageManager.PERMISSION_GRANTED
    } catch (t: Throwable) {
        false
    }

    fun requestPermission() {
        try {
            if (isAvailable()) {
                Shizuku.requestPermission(REQUEST_CODE)
            }
        } catch (_: Throwable) {
        }
    }

    fun addPermissionListener(listener: Shizuku.OnRequestPermissionResultListener) {
        try {
            Shizuku.addRequestPermissionResultListener(listener)
        } catch (_: Throwable) {
        }
    }

    fun removePermissionListener(listener: Shizuku.OnRequestPermissionResultListener) {
        try {
            Shizuku.removeRequestPermissionResultListener(listener)
        } catch (_: Throwable) {
        }
    }

    fun shell(command: String): ShellResult {
        if (command.isBlank()) {
            return ShellResult(-1, "", "empty command")
        }
        if (!hasPermission()) {
            return ShellResult(-1, "", "Shizuku permission is not granted")
        }

        val service = try {
            Shizuku.getBinder()?.let { IShizukuService.Stub.asInterface(it) }
        } catch (t: Throwable) {
            null
        } ?: return ShellResult(-1, "", "Shizuku service unavailable")

        val process = try {
            service.newProcess(arrayOf("sh", "-c", command), null, null)
        } catch (t: Throwable) {
            return ShellResult(-1, "", t.message ?: "cannot start shell")
        }

        val stdout = StringBuilder()
        val stderr = StringBuilder()
        val outReader = Thread {
            runCatching {
                ParcelFileDescriptor.AutoCloseInputStream(process.inputStream)
                    .bufferedReader()
                    .forEachLine { stdout.appendLine(it) }
            }
        }
        val errReader = Thread {
            runCatching {
                ParcelFileDescriptor.AutoCloseInputStream(process.errorStream)
                    .bufferedReader()
                    .forEachLine { stderr.appendLine(it) }
            }
        }
        val waiter = Thread {
            runCatching { process.waitFor() }
        }
        outReader.start()
        errReader.start()
        waiter.start()

        waiter.join(30_000)
        val code = if (waiter.isAlive) {
            runCatching { process.destroy() }
            -1
        } else {
            runCatching { process.exitValue() }.getOrDefault(-1)
        }
        outReader.join(2_000)
        errReader.join(2_000)
        return ShellResult(code, stdout.toString().trimEnd(), stderr.toString().trimEnd())
    }

    fun grant(permission: String): ShellResult {
        require(permission in grantablePermissions) { "permission not allowed: $permission" }
        return shell("pm grant ${BuildConfig.APPLICATION_ID} $permission")
    }

    fun grantOverlay(): ShellResult =
        shell("appops set ${BuildConfig.APPLICATION_ID} SYSTEM_ALERT_WINDOW allow")

    fun grantNotifications(): ShellResult =
        grant("android.permission.POST_NOTIFICATIONS")

    fun launchPackage(packageName: String): ShellResult =
        shell("monkey -p $packageName -c android.intent.category.LAUNCHER 1")
}
