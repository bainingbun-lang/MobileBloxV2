package com.mobileblox.v2.bridge

import com.google.gson.JsonObject
import okhttp3.MediaType.Companion.toMediaTypeOrNull
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import java.util.Locale
import java.util.concurrent.TimeUnit

data class ProxyResponse(
    val body: String,
    val code: Int,
    val reason: String,
    val version: String,
    val headers: Map<String, String>,
) {
    fun toJson(): String {
        val json = JsonObject()
        json.addProperty("b", body)
        json.addProperty("c", code)
        json.addProperty("r", reason)
        json.addProperty("v", version)
        val headerJson = JsonObject()
        headers.forEach { (key, value) -> headerJson.addProperty(key, value) }
        json.add("h", headerJson)
        return json.toString()
    }
}

/**
 * Portable counterpart of ExternalExecutor's "request" command.
 */
class HttpProxy(private val client: OkHttpClient = defaultClient()) {
    fun request(
        url: String,
        method: String,
        body: String?,
        headers: Map<String, String>,
    ): ProxyResponse {
        val builder = Request.Builder().url(url)
        headers.forEach { (key, value) ->
            runCatching { builder.addHeader(key, value) }
        }

        val verb = method.trim().uppercase().ifEmpty { "GET" }
        val contentType = headers.entries
            .firstOrNull { it.key.equals("Content-Type", ignoreCase = true) }
            ?.value
            ?.toMediaTypeOrNull()
            ?: "application/json".toMediaTypeOrNull()

        val requestBody = when (verb) {
            "GET", "HEAD" -> null
            else -> (body ?: "").toRequestBody(contentType)
        }
        builder.method(verb, requestBody)

        client.newCall(builder.build()).execute().use { response ->
            val responseHeaders = response.headers
                .toMultimap()
                .entries
                .associate { (name, values) ->
                    name.lowercase(Locale.US) to values.joinToString(", ")
                }
            return ProxyResponse(
                body = response.body?.string().orEmpty(),
                code = response.code,
                reason = response.message,
                version = response.protocol.toString(),
                headers = responseHeaders,
            )
        }
    }

    companion object {
        fun defaultClient(): OkHttpClient = OkHttpClient.Builder()
            .connectTimeout(15, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .followRedirects(true)
            .build()
    }
}
