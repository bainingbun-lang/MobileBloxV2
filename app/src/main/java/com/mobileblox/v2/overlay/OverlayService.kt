package com.mobileblox.v2.overlay

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.graphics.PixelFormat
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.provider.Settings
import androidx.appcompat.view.ContextThemeWrapper
import android.view.Gravity
import android.view.KeyEvent
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewConfiguration
import android.view.WindowManager
import android.widget.EditText
import android.widget.ScrollView
import android.widget.TextView
import androidx.core.app.NotificationCompat
import com.mobileblox.v2.R
import com.mobileblox.v2.luau.LuauBridge
import kotlin.math.abs

/**
 * Foreground service that keeps a draggable bubble on top of other apps
 * (including Roblox). Tapping the bubble opens a centered floating executor.
 */
class OverlayService : Service() {

    private lateinit var windowManager: WindowManager
    private lateinit var params: WindowManager.LayoutParams
    private var bubble: View? = null
    private var panel: View? = null

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
        hidePanel()
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
            overlayType(),
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
                        showPanel()
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

    private fun showPanel() {
        if (panel != null) {
            return
        }
        val themed = ContextThemeWrapper(this, R.style.Theme_MobileBlox)
        val view = LayoutInflater.from(themed).inflate(R.layout.overlay_executor, null)
        val editor = view.findViewById<EditText>(R.id.overlay_editor)
        val output = view.findViewById<TextView>(R.id.overlay_output)
        val outputScroll = view.findViewById<ScrollView>(R.id.overlay_output_scroll)

        val panelParams = WindowManager.LayoutParams(
            WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.MATCH_PARENT,
            overlayType(),
            WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
            PixelFormat.TRANSLUCENT,
        ).apply {
            gravity = Gravity.CENTER
            softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE or
                WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE
        }

        view.findViewById<View>(R.id.overlay_root).setOnClickListener { hidePanel() }
        view.findViewById<View>(R.id.overlay_panel).setOnClickListener { }
        view.findViewById<View>(R.id.overlay_close).setOnClickListener { hidePanel() }
        view.findViewById<View>(R.id.overlay_clear).setOnClickListener {
            editor.setText("")
            output.text = ""
        }
        view.findViewById<View>(R.id.overlay_execute).setOnClickListener {
            runScript(editor, output, outputScroll)
        }

        view.isFocusableInTouchMode = true
        view.requestFocus()
        view.setOnKeyListener { _, keyCode, event ->
            if (keyCode == KeyEvent.KEYCODE_BACK && event.action == KeyEvent.ACTION_UP) {
                hidePanel()
                true
            } else {
                false
            }
        }

        runCatching {
            windowManager.addView(view, panelParams)
            panel = view
            bubble?.visibility = View.GONE
        }.onFailure {
            panel = null
        }
    }

    private fun hidePanel() {
        panel?.let { view ->
            runCatching { windowManager.removeView(view) }
        }
        panel = null
        bubble?.visibility = View.VISIBLE
    }

    private fun runScript(editor: EditText, output: TextView, outputScroll: ScrollView) {
        val source = editor.text.toString()
        if (source.isBlank()) {
            output.text = getString(R.string.empty_script)
            return
        }
        output.text = getString(R.string.running)
        Thread {
            val result = if (!LuauBridge.available) {
                getString(R.string.luau_missing)
            } else {
                runCatching { LuauBridge.run(source) }
                    .getOrElse { "error: ${it.message}" }
            }
            Handler(Looper.getMainLooper()).post {
                output.text = result.trimEnd().ifEmpty { "(sin salida)" }
                outputScroll.post { outputScroll.fullScroll(View.FOCUS_DOWN) }
            }
        }.start()
    }

    private fun overlayType(): Int =
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
        } else {
            @Suppress("DEPRECATION")
            WindowManager.LayoutParams.TYPE_PHONE
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
