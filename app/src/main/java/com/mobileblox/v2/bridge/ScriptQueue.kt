package com.mobileblox.v2.bridge

/**
 * Ordered script hand-off between the UI and the bridge clients.
 * Mirrors ExternalExecutor's "listen" semantics: every client receives each
 * script exactly once, identified by a monotonically increasing order.
 */
class ScriptQueue {
    private var script: String = ""
    private var order: Long = 0
    private val delivered = HashMap<String, Long>()

    @Synchronized
    fun execute(source: String): Long {
        script = source
        order += 1
        return order
    }

    @Synchronized
    fun poll(clientId: String): String {
        val last = delivered[clientId]
        val fresh = last == null || last < order
        delivered[clientId] = order
        return if (fresh) script else ""
    }

    @Synchronized
    fun currentOrder(): Long = order

    @Synchronized
    fun currentScript(): String = script
}
