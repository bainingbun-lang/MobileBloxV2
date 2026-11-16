package com.mobileblox.v2

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import timber.log.Timber

class MainFragment : Fragment() {

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return inflater.inflate(R.layout.fragment_main, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Placeholder UI showing the planned layout
        val statusText = view.findViewById<TextView>(R.id.status_text)
        statusText.text = "MobileBlox V2\n\n" +
            "Phase 2: ARM64 Compatibility - In Progress\n\n" +
            "✓ CMake migration\n" +
            "✓ Gradle Android project\n" +
            "✓ Native library structure\n\n" +
            "Next: Build dependencies for ARM64\n" +
            "Then: ImGui integration + Editor + Explorer + AI"

        Timber.d("MainFragment created")
    }
}