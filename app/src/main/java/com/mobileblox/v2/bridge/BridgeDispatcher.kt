package com.mobileblox.v2.bridge

import com.google.gson.Gson
import com.google.gson.JsonObject

data class BridgeResult(val status: Int, val body: String)

/**
 * Command dispatcher for the local bridge. Command names and settings keys are
 * kept compatible with ExternalExecutor so existing clients can be reused.
 */
class BridgeDispatcher(
    private val files: FileRepository,
    private val http: HttpProxy,
    private val queue: ScriptQueue,
    private val identify: () -> Map<String, Any?>,
) {
    private val gson = Gson()

    fun handle(command: BridgeCommand): BridgeResult = try {
        when (command.type) {
            "ping" -> ok("pong")
            "identify" -> ok(gson.toJson(identify()))
            "request" -> ok(proxy(command))
            "readfile" -> ok(files.read(command.settings.string("path")))
            "writefile" -> {
                files.write(command.settings.string("path"), command.payload)
                ok("ok")
            }
            "listfiles" -> ok(gson.toJson(files.list(command.settings.string("path"))))
            "exists" -> ok(files.exists(command.settings.string("path")).toString())
            "listen" -> ok(queue.poll(command.clientId))
            "compile", "setscriptbytecode" ->
                BridgeResult(501, error("'${command.type}' requires the native Luau module (not built yet)"))
            else -> BridgeResult(404, error("unknown command '${command.type}'"))
        }
    } catch (e: IllegalArgumentException) {
        BridgeResult(400, error(e.message ?: "invalid request"))
    } catch (e: Exception) {
        BridgeResult(500, error(e.message ?: e.javaClass.simpleName))
    }

    private fun proxy(command: BridgeCommand): String {
        val url = command.settings.string("l")
        require(url.isNotBlank()) { "missing 'l' (url)" }
        val method = command.settings.string("m").ifBlank { "GET" }
        val body = command.settings.get("b")?.takeIf { !it.isJsonNull }?.asString
        val headers = command.settings.getAsJsonObject("h")
            ?.entrySet()
            ?.associate { (key, value) -> key to value.asString }
            .orEmpty()
        return http.request(url, method, body, headers).toJson()
    }

    private fun ok(body: String) = BridgeResult(200, body)

    private fun error(message: String): String =
        JsonObject().apply { addProperty("error", message) }.toString()

    private fun JsonObject.string(key: String): String =
        get(key)?.takeIf { !it.isJsonNull }?.asString.orEmpty()
}
