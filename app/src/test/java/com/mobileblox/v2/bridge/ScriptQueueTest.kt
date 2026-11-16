package com.mobileblox.v2.bridge

import org.junit.Assert.assertEquals
import org.junit.Test

class ScriptQueueTest {

    private val queue = ScriptQueue()

    @Test
    fun newClientReceivesCurrentScript() {
        queue.execute("print('a')")
        assertEquals("print('a')", queue.poll("client-1"))
    }

    @Test
    fun sameOrderIsNotRedelivered() {
        queue.execute("a")
        assertEquals("a", queue.poll("client-1"))
        assertEquals("", queue.poll("client-1"))
    }

    @Test
    fun eachNewScriptIsDeliveredOncePerClient() {
        queue.execute("a")
        queue.poll("client-1")
        queue.execute("b")
        assertEquals("b", queue.poll("client-1"))
        assertEquals("", queue.poll("client-1"))
    }

    @Test
    fun lateClientGetsLatestScript() {
        queue.execute("a")
        queue.execute("b")
        assertEquals("b", queue.poll("client-2"))
    }
}
