package com.mobileblox.v2

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.mobileblox.v2.bridge.BridgeService
import com.mobileblox.v2.bridge.FileRepository
import timber.log.Timber
import java.io.File
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainFragment : Fragment() {

    private lateinit var editor: EditText
    private lateinit var statusText: TextView
    private lateinit var bridgeInfo: TextView
    private lateinit var logText: TextView
    private val files by lazy { FileRepository(File(requireContext().filesDir, "MobileBlox")) }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return inflater.inflate(R.layout.fragment_main, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        editor = view.findViewById(R.id.script_editor)
        statusText = view.findViewById(R.id.status_text)
        bridgeInfo = view.findViewById(R.id.bridge_info)
        logText = view.findViewById(R.id.log_text)

        view.findViewById<Button>(R.id.btn_execute).setOnClickListener { enqueueScript() }
        view.findViewById<Button>(R.id.btn_clear).setOnClickListener {
            editor.setText("")
            log("Editor limpiado")
        }
        view.findViewById<Button>(R.id.btn_save).setOnClickListener { saveScript() }
        view.findViewById<Button>(R.id.btn_open).setOnClickListener { openScript() }

        renderStatus()
        log("Listo.")
        Timber.d("MainFragment created")
    }

    override fun onResume() {
        super.onResume()
        if (::statusText.isInitialized) renderStatus()
    }

    private fun renderStatus() {
        statusText.text = "MobileBlox V2 ${BuildConfig.VERSION_NAME}"
        bridgeInfo.text = if (BridgeService.running) {
            "Bridge: http://127.0.0.1:${BridgeService.PORT}/handle\nToken: ${BridgeService.token}"
        } else {
            "Bridge: inactivo" + (BridgeService.lastError?.let { " ($it)" } ?: "")
        }
    }

    private fun enqueueScript() {
        val source = editor.text.toString()
        if (source.isBlank()) {
            toast("Script vacío")
            return
        }
        val order = BridgeService.scriptQueue.execute(source)
        log("Encolado #$order (${source.lines().size} líneas)")
        toast("Script encolado #$order")
    }

    private fun saveScript() {
        runCatching { files.write(SCRIPT_PATH, editor.text.toString()) }
            .onSuccess { log("Guardado en ${files.rootPath()}/$SCRIPT_PATH") }
            .onFailure { log("Error al guardar: ${it.message}") }
    }

    private fun openScript() {
        runCatching { files.read(SCRIPT_PATH) }
            .onSuccess {
                editor.setText(it)
                log("Abierto $SCRIPT_PATH")
            }
            .onFailure { log("No se pudo abrir: ${it.message}") }
    }

    private fun log(message: String) {
        val timestamp = SimpleDateFormat("HH:mm:ss", Locale.US).format(Date())
        val line = "[$timestamp] $message"
        logText.append(if (logText.text.isNullOrEmpty()) line else "\n$line")
    }

    private fun toast(message: String) =
        Toast.makeText(requireContext(), message, Toast.LENGTH_SHORT).show()

    companion object {
        private const val SCRIPT_PATH = "scripts/main.lua"
    }
}
