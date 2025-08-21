package com.scorbutics.rubyvm

class RubyScript private constructor(private val nativePtr: Long): AutoCloseable {

    companion object {
        fun fromContent(content: String): RubyScript {
            val ptr = RubyVMNative.createScript(content)
            if (ptr == 0L) {
                throw RuntimeException("Failed to create RubyScript")
            }
            return RubyScript(ptr)
        }

        fun fromAssets(assetManager: android.content.res.AssetManager, scriptName: String): RubyScript {
            // Read content from assets and create from content
            val content = assetManager.open(scriptName).bufferedReader().use { it.readText() }
            return fromContent(content)
        }
    }

    internal fun getNativePtr(): Long = nativePtr

    private fun destroy() {
        if (nativePtr != 0L) {
            RubyVMNative.destroyScript(nativePtr)
        }
    }

    override fun close() = destroy()
}
