package com.mobileblox.v2

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.lifecycle.lifecycleScope
import com.google.android.material.snackbar.Snackbar
import kotlinx.coroutines.launch
import timber.log.Timber

class MainActivity : AppCompatActivity() {

    private lateinit var toolbar: Toolbar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        toolbar = findViewById(R.id.toolbar)
        setSupportActionBar(toolbar)

        // Initialize UI fragments (Editor, Explorer, AI Chat, Console)
        // For now, show placeholder
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_container, MainFragment())
            .commit()

        Timber.d("MainActivity created")
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