package com.mobileblox.v2

import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.MultiAutoCompleteTextView
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.mobileblox.v2.bridge.BridgeService
import com.mobileblox.v2.bridge.FileRepository
import com.mobileblox.v2.editor.CodeAutocomplete
import com.mobileblox.v2.editor.ExampleScripts
import com.mobileblox.v2.editor.ScriptHistory
import com.mobileblox.v2.launcher.RobloxLauncher
import com.mobileblox.v2.luau.LuauBridge
import com.mobileblox.v2.overlay.OverlayService
import com.mobileblox.v2.shizuku.ShizukuManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import rikka.shizuku.Shizuku
import timber.log.Timber
import java.io.File
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainFragment : Fragment() {

    private lateinit var editor: MultiAutoCompleteTextView
    private lateinit var statusText: TextView
    private lateinit var bridgeInfo: TextView
    private lateinit var shizukuStatus: TextView
    private lateinit var logText: TextView
    private val files by lazy { FileRepository(File(requireContext().filesDir, "MobileBlox")) }

    private val permissionListener = Shizuku.OnRequestPermissionResultListener { requestCode, grantResult ->
        if (requestCode == ShizukuManager.REQUEST_CODE) {
            activity?.runOnUiThread {
                renderShizuku()
                log(
                    if (grantResult == PackageManager.PERMISSION_GRANTED) {
                        "Shizuku autorizado"
                    } else {
                        "Shizuku denegado"
                    },
                )
            }
        }
    }

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
        CodeAutocomplete.attach(editor, requireContext())
        statusText = view.findViewById(R.id.status_text)
        bridgeInfo = view.findViewById(R.id.bridge_info)
        shizukuStatus = view.findViewById(R.id.shizuku_status)
        logText = view.findViewById(R.id.log_text)

        view.findViewById<Button>(R.id.btn_execute).setOnClickListener { executeScript() }
        view.findViewById<Button>(R.id.btn_clear).setOnClickListener {
            editor.setText("")
            log("Editor limpiado")
        }
        view.findViewById<Button>(R.id.btn_history).setOnClickListener { showHistory() }
        view.findViewById<Button>(R.id.btn_examples).setOnClickListener { showExamples() }
        view.findViewById<Button>(R.id.btn_save).setOnClickListener { saveScript() }
        view.findViewById<Button>(R.id.btn_open).setOnClickListener { openScript() }
        view.findViewById<Button>(R.id.btn_launch_roblox).setOnClickListener { launchRoblox() }
        view.findViewById<Button>(R.id.btn_pip).setOnClickListener {
            if ((activity as? MainActivity)?.enterPip() != true) {
                log("PiP no disponible en este dispositivo")
            }
        }
        view.findViewById<Button>(R.id.btn_shizuku).setOnClickListener { requestShizuku() }
        view.findViewById<Button>(R.id.btn_overlay).setOnClickListener { grantOverlay() }
        view.findViewById<Button>(R.id.btn_bubble).setOnClickListener { toggleBubble() }

        renderStatus()
        renderShizuku()
        log("Listo.")
        Timber.d("MainFragment created")
    }

    override fun onStart() {
        super.onStart()
        ShizukuManager.addPermissionListener(permissionListener)
    }

    override fun onStop() {
        ShizukuManager.removePermissionListener(permissionListener)
        super.onStop()
    }

    override fun onResume() {
        super.onResume()
        if (::statusText.isInitialized) {
            renderStatus()
            renderShizuku()
        }
    }

    private fun renderStatus() {
        statusText.text = "MobileBlox V2 ${BuildConfig.VERSION_NAME}"
        bridgeInfo.text = if (BridgeService.running) {
            "Bridge: http://127.0.0.1:${BridgeService.PORT}/handle\nToken: ${BridgeService.token}"
        } else {
            "Bridge: inactivo" + (BridgeService.lastError?.let { " ($it)" } ?: "")
        }
    }

    private fun renderShizuku() {
        shizukuStatus.text = when {
            !ShizukuManager.isAvailable() -> getString(R.string.shizuku_missing)
            ShizukuManager.hasPermission() -> getString(R.string.shizuku_ready)
            else -> getString(R.string.shizuku_permission_needed)
        }
    }

    private fun executeScript() {
        val source = editor.text.toString()
        if (source.isBlank()) {
            toast("Script vacío")
            return
        }
        ScriptHistory.add(requireContext(), source)
        val order = BridgeService.scriptQueue.execute(source)
        log("Encolado #$order (${source.lines().size} líneas)")

        if (!LuauBridge.available) {
            log(getString(R.string.luau_missing))
            return
        }
        log("Ejecutando en la app…")
        viewLifecycleOwner.lifecycleScope.launch {
            val output = withContext(Dispatchers.Default) {
                runCatching { LuauBridge.run(source) }
                    .getOrElse { "error: ${it.message}" }
            }
            log(output.trimEnd().ifEmpty { "(sin salida)" })
        }
    }

    private fun showHistory() {
        val entries = ScriptHistory.all(requireContext())
        if (entries.isEmpty()) {
            toast(getString(R.string.history_empty))
            return
        }
        AlertDialog.Builder(requireContext())
            .setTitle(R.string.history)
            .setItems(entries.map { preview(it) }.toTypedArray()) { _, which ->
                editor.setText(entries[which])
            }
            .setNeutralButton(R.string.clear_history) { _, _ ->
                ScriptHistory.clear(requireContext())
                log("Historial borrado")
            }
            .setNegativeButton(R.string.close, null)
            .show()
    }

    private fun showExamples() {
        val examples = ExampleScripts.all
        AlertDialog.Builder(requireContext())
            .setTitle(R.string.examples)
            .setItems(examples.map { it.first }.toTypedArray()) { _, which ->
                editor.setText(examples[which].second)
            }
            .setNegativeButton(R.string.close, null)
            .show()
    }

    private fun preview(script: String): String {
        val firstLine = script.lineSequence().firstOrNull()?.trim().orEmpty()
        return firstLine.take(70).ifEmpty { "(script)" }
    }

    private fun launchRoblox() {
        if (!RobloxLauncher.isInstalled(requireContext())) {
            toast(getString(R.string.roblox_missing))
            return
        }
        log("Abriendo Roblox…")
        val activity = activity as? MainActivity
        if (activity != null) {
            activity.launchRoblox()
        } else {
            RobloxLauncher.launch(requireContext())
        }
    }

    private fun requestShizuku() {
        if (!ShizukuManager.isAvailable()) {
            toast(getString(R.string.shizuku_missing))
            return
        }
        if (ShizukuManager.hasPermission()) {
            toast(getString(R.string.shizuku_ready))
            return
        }
        log("Solicitando permiso de Shizuku…")
        ShizukuManager.requestPermission()
    }

    private fun grantOverlay() {
        if (!ShizukuManager.hasPermission()) {
            toast("Autoriza Shizuku primero")
            return
        }
        viewLifecycleOwner.lifecycleScope.launch {
            log("Concediendo permiso de overlay…")
            val result = withContext(Dispatchers.IO) { ShizukuManager.grantOverlay() }
            log("Overlay: ${result.stdout.ifBlank { result.stderr.ifBlank { "código ${result.code}" } }}")
            if (Settings.canDrawOverlays(requireContext())) {
                log("Permiso de overlay concedido")
            } else {
                log("Abre Ajustes > Permisos especiales si no se concedió")
            }
        }
    }

    private fun toggleBubble() {
        if (Settings.canDrawOverlays(requireContext())) {
            startBubble()
            return
        }
        if (ShizukuManager.hasPermission()) {
            viewLifecycleOwner.lifecycleScope.launch {
                val result = withContext(Dispatchers.IO) { ShizukuManager.grantOverlay() }
                log("Overlay: ${result.stdout.ifBlank { result.stderr }}")
                if (Settings.canDrawOverlays(requireContext())) {
                    startBubble()
                } else {
                    openOverlaySettings()
                }
            }
        } else {
            openOverlaySettings()
        }
    }

    private fun startBubble() {
        ContextCompat.startForegroundService(
            requireContext(),
            Intent(requireContext(), OverlayService::class.java),
        )
        log("Burbuja flotante iniciada")
    }

    private fun openOverlaySettings() {
        runCatching {
            startActivity(
                Intent(
                    Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                    Uri.parse("package:${requireContext().packageName}"),
                ),
            )
        }
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
