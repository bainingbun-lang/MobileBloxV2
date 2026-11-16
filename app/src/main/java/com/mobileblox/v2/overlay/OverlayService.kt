package com.mobileblox.v2.overlay

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.graphics.PixelFormat
import android.os.Build
import android.os.IBinder
import android.provider.Settings
import android.view.Gravity
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewConfiguration
import android.view.WindowManager
import androidx.core.app.NotificationCompat
import com.mobileblox.v2.MainActivity
import com.mobileblox.v2.R
import kotlin.math.abs

/**
 * Foreground service that keeps a draggable bubble on top of other apps
 * (including Roblox). Tapping the bubble opens the in-app executor.
 */
class OverlayService : Service() {

    private lateinit var windowManager: WindowManager
    private lateinit var params: WindowManager.LayoutParams
    private var bubble: View? = null

    private var initialX = 0
    private var initialY = 0
    private var initialTouchX = 0f
    private var initialTouchY = 0f
    private var moved = false

    override fun onCreate() {
        super.onCreate()
        windowManager = getSystemService(WINDOW_SERVICE) as WindowManager
        startForeground(NOTIFICATION_ID, buildNotification())

        if (!Settings.canDrawOverlays(this)) {
            stopSelf()
            return
        }
        addBubble()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int = START_STICKY

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        bubble?.let { view ->
            runCatching { windowManager.removeView(view) }
        }
        bubble = null
        super.onDestroy()
    }

    private fun addBubble() {
        val view = LayoutInflater.from(this).inflate(R.layout.overlay_bubble, null)
        params = WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            } else {
                @Suppress("DEPRECATION")
                WindowManager.LayoutParams.TYPE_PHONE
            },
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT,
        ).apply {
            gravity = Gravity.TOP or Gravity.START
            x = 32
            y = 280
        }

        val touchSlop = ViewConfiguration.get(this).scaledTouchSlop
        view.setOnTouchListener { _, event ->
            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    initialX = params.x
                    initialY = params.y
                    initialTouchX = event.rawX
                    initialTouchY = event.rawY
                    moved = false
                    true
                }

                MotionEvent.ACTION_MOVE -> {
                    val dx = (event.rawX - initialTouchX).toInt()
                    val dy = (event.rawY - initialTouchY).toInt()
                    if (abs(dx) > touchSlop || abs(dy) > touchSlop) {
                        moved = true
                    }
                    params.x = initialX + dx
                    params.y = initialY + dy
                    runCatching { windowManager.updateViewLayout(view, params) }
                    true
                }

                MotionEvent.ACTION_UP -> {
                    if (!moved) {
                        openExecutor()
                    }
                    true
                }

                else -> false
            }
        }

        view.findViewById<View>(R.id.bubble_close).setOnClickListener { stopSelf() }

        runCatching {
            windowManager.addView(view, params)
            bubble = view
        }.onFailure { stopSelf() }
    }

    private fun openExecutor() {
        val intent = Intent(this, MainActivity::class.java).apply {
            addFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK or
                    Intent.FLAG_ACTIVITY_SINGLE_TOP or
                    Intent.FLAG_ACTIVITY_REORDER_TO_FRONT,
            )
        }
        startActivity(intent)
    }

    private fun buildNotification(): Notification {
        val channelId = CHANNEL_ID
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val manager = getSystemService(NotificationManager::class.java)
            if (manager.getNotificationChannel(channelId) == null) {
                manager.createNotificationChannel(
                    NotificationChannel(
                        channelId,
                        getString(R.string.overlay_channel),
                        NotificationManager.IMPORTANCE_LOW,
                    ),
                )
            }
        }
        return NotificationCompat.Builder(this, channelId)
            .setSmallIcon(R.drawable.ic_terminal)
            .setContentTitle(getString(R.string.overlay_running))
            .setContentText(getString(R.string.overlay_hint))
            .setOngoing(true)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()
    }

    companion object {
        private const val CHANNEL_ID = "floating_executor"
        private const val NOTIFICATION_ID = 0x0B10
    }
}
