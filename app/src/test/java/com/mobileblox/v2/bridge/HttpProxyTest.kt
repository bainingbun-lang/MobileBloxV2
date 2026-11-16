package com.mobileblox.v2.bridge

import okhttp3.mockwebserver.MockResponse
import okhttp3.mockwebserver.MockWebServer
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test

class HttpProxyTest {

    @get:Rule
    val server = MockWebServer()

    @Test
    fun proxiesGetAndMapsResponse() {
        server.enqueue(
            MockResponse()
                .setResponseCode(201)
                .setHeader("X-Test", "yes")
                .setBody("hello")
        )

        val response = HttpProxy().request(
            server.url("/x").toString(),
            "GET",
            null,
            mapOf("Accept" to "text/plain"),
        )

        assertEquals("hello", response.body)
        assertEquals(201, response.code)
        assertEquals("yes", response.headers["X-Test"])

        val request = server.takeRequest()
        assertEquals("/x", request.path)
        assertEquals("text/plain", request.getHeader("Accept"))
    }

    @Test
    fun proxiesPostBody() {
        server.enqueue(MockResponse().setBody("ok"))

        val response = HttpProxy().request(
            server.url("/p").toString(),
            "POST",
            "{\"a\":1}",
            mapOf("Content-Type" to "application/json"),
        )

        assertEquals("ok", response.body)
        val request = server.takeRequest()
        assertEquals("POST", request.method)
        assertEquals("{\"a\":1}", request.body.readUtf8())
    }
}
