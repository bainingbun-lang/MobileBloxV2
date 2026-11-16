package com.mobileblox.v2.luau

/**
 * JNI facade over the Luau compiler + VM built from https://github.com/luau-lang/luau.
 *
 * The native module is built for arm64-v8a by CI. If it is missing (for example a
 * local build without the NDK), [available] is false and callers get a clear error.
 */
object LuauBridge {

    val available: Boolean by lazy {
        try {
            System.loadLibrary("mobileblox_luau")
            true
        } catch (t: UnsatisfiedLinkError) {
            false
        }
    }

    fun compile(source: String): ByteArray {
        check(available) { "Luau native library is not available in this build" }
        return nativeCompile(withPrelude(source)) ?: throw IllegalStateException("Luau compile failed")
    }

    fun run(source: String): String {
        check(available) { "Luau native library is not available in this build" }
        return nativeRun(withPrelude(source)) ?: "error: native run failed"
    }

    private fun withPrelude(source: String): String =
        RobloxApiPrelude.SOURCE + "\n" + source

    fun runBytecode(bytecode: ByteArray): String {
        check(available) { "Luau native library is not available in this build" }
        return nativeRunBytecode(bytecode) ?: "error: native run failed"
    }

    private external fun nativeCompile(source: String): ByteArray?
    private external fun nativeRun(source: String): String?
    private external fun nativeRunBytecode(bytecode: ByteArray): String?
}
