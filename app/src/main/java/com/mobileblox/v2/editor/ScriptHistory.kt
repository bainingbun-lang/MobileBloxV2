package com.mobileblox.v2.editor

import android.content.Context
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken

/** Persists the most recent executed scripts. */
object ScriptHistory {

    private const val PREFS = "mobileblox_history"
    private const val KEY = "entries"
    private const val LIMIT = 20
    private val gson = Gson()

    fun all(context: Context): List<String> {
        val raw = context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .getString(KEY, null) ?: return emptyList()
        return runCatching {
            val type = object : TypeToken<List<String>>() {}.type
            val parsed: List<String>? = gson.fromJson(raw, type)
            parsed ?: emptyList()
        }.getOrDefault(emptyList())
    }

    fun add(context: Context, script: String) {
        val trimmed = script.trim()
        if (trimmed.isEmpty()) {
            return
        }
        val entries = all(context).toMutableList()
        entries.remove(trimmed)
        entries.add(0, trimmed)
        while (entries.size > LIMIT) {
            entries.removeAt(entries.lastIndex)
        }
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .edit()
            .putString(KEY, gson.toJson(entries))
            .apply()
    }

    fun clear(context: Context) {
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .edit()
            .remove(KEY)
            .apply()
    }
}
