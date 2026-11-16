package com.mobileblox.v2.bridge

import com.google.gson.JsonObject
import com.google.gson.JsonParser

/**
 * Wire-compatible with ExternalExecutor's bridge protocol:
 *
 *     <type>\n<clientId>\n<json settings>\n<payload>
 */
data class BridgeCommand(
    val type: String,
    val clientId: String,
    val settings: JsonObject,
    val payload: String,
)

object BridgeCommandParser {
    fun parse(body: String): BridgeCommand {
        val lines = body.split('\n')
        val type = lines.getOrNull(0)?.trim().orEmpty()
        val clientId = lines.getOrNull(1)?.trim().orEmpty()
        val settings = parseSettings(lines.getOrNull(2))
        val payload = if (lines.size > 3) lines.subList(3, lines.size).joinToString("\n") else ""
        return BridgeCommand(type, clientId, settings, payload)
    }

    private fun parseSettings(raw: String?): JsonObject {
        val text = raw?.trim().orEmpty()
        if (text.isEmpty()) return JsonObject()
        return runCatching { JsonParser.parseString(text).asJsonObject }.getOrElse { JsonObject() }
    }
}
