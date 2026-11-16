package com.mobileblox.v2.bridge

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class BridgeCommandParserTest {

    @Test
    fun parsesTypeClientSettingsAndPayload() {
        val command = BridgeCommandParser.parse(
            "request\n42\n{\"l\":\"https://example.com\",\"m\":\"GET\"}\nline1\nline2"
        )
        assertEquals("request", command.type)
        assertEquals("42", command.clientId)
        assertEquals("https://example.com", command.settings.get("l").asString)
        assertEquals("line1\nline2", command.payload)
    }

    @Test
    fun malformedSettingsBecomeEmptyObject() {
        val command = BridgeCommandParser.parse("ping\n0\nnot-json")
        assertTrue(command.settings.entrySet().isEmpty())
        assertEquals("", command.payload)
    }

    @Test
    fun missingFieldsAreSafe() {
        val command = BridgeCommandParser.parse("ping")
        assertEquals("ping", command.type)
        assertEquals("", command.clientId)
        assertEquals("", command.payload)
    }
}
