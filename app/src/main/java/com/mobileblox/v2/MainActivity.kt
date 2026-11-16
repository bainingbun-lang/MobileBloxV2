package com.mobileblox.v2

import android.app.PictureInPictureParams
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Rational
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import com.google.android.material.snackbar.Snackbar
import com.mobileblox.v2.launcher.RobloxLauncher
import timber.log.Timber

class MainActivity : AppCompatActivity() {

    private lateinit var toolbar: Toolbar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        toolbar = findViewById(R.id.toolbar)
        setSupportActionBar(toolbar)

        supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_container, MainFragment())
            .commit()

        Timber.d("MainActivity created")
    }

    fun enterPip(): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            return false
        }
        return try {
            enterPictureInPictureMode(
                PictureInPictureParams.Builder()
                    .setAspectRatio(Rational(9, 16))
                    .build(),
            )
            true
        } catch (t: Throwable) {
            Timber.w(t, "Cannot enter PiP")
            false
        }
    }

    fun launchRoblox() {
        if (!RobloxLauncher.isInstalled(this)) {
            Toast.makeText(this, R.string.roblox_missing, Toast.LENGTH_SHORT).show()
            return
        }
        enterPip()
        Handler(Looper.getMainLooper()).postDelayed({
            if (!RobloxLauncher.launch(this)) {
                Toast.makeText(this, R.string.roblox_missing, Toast.LENGTH_SHORT).show()
            }
        }, 350)
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_new_file -> {
                Snackbar.make(findViewById(android.R.id.content), "New file - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            R.id.action_open_file -> {
                Snackbar.make(findViewById(android.R.id.content), "Open file - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            R.id.action_settings -> {
                Snackbar.make(findViewById(android.R.id.content), "Settings - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            R.id.action_ai_assistant -> {
                Snackbar.make(findViewById(android.R.id.content), "AI Assistant - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            R.id.action_explorer -> {
                Snackbar.make(findViewById(android.R.id.content), "Explorer toggle - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            R.id.action_console -> {
                Snackbar.make(findViewById(android.R.id.content), "Console toggle - TODO", Snackbar.LENGTH_SHORT).show()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        Timber.d("MainActivity destroyed")
    }
}
