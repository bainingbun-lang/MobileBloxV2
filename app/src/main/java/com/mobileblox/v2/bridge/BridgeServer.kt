package com.mobileblox.v2.bridge

import fi.iki.elonen.NanoHTTPD

/**
 * Localhost-only HTTP endpoint compatible with ExternalExecutor's `/handle`.
 * When [token] is set, clients must send it as `X-Bridge-Token`.
 */
class BridgeServer(
    port: Int,
    private val dispatcher: BridgeDispatcher,
    private val token: String? = null,
) : NanoHTTPD("127.0.0.1", port) {

    override fun serve(session: IHTTPSession): Response {
        if (session.method != Method.POST || session.uri != "/handle") {
            return newFixedLengthResponse(Response.Status.NOT_FOUND, MIME_PLAINTEXT, "not found")
        }
        if (!token.isNullOrEmpty() && session.headers["x-bridge-token"] != token) {
            return newFixedLengthResponse(Response.Status.UNAUTHORIZED, MIME_PLAINTEXT, "unauthorized")
        }

        val body = try {
            val parsed = HashMap<String, String>()
            session.parseBody(parsed)
            parsed["postData"].orEmpty()
        } catch (e: Exception) {
            return newFixedLengthResponse(Response.Status.BAD_REQUEST, MIME_PLAINTEXT, "invalid body")
        }

        val result = dispatcher.handle(BridgeCommandParser.parse(body))
        val status = Response.Status.lookup(result.status) ?: Response.Status.OK
        return newFixedLengthResponse(status, MIME_PLAINTEXT, result.body)
    }
}
