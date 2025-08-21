package com.scorbutics.rubyvm


fun interface CompletionCallback {
  fun complete(result: Int)
}

abstract class RubyInterpreter internal constructor(
    private val nativePtr: Long,
) : LogListener, AutoCloseable {

    companion object {
        fun create(
          applicationPath: String,
          location: ScriptCurrentLocation,
          logListener: LogListener
        ): RubyInterpreter {
            return object : RubyInterpreter(0L) {
                private val actualPtr: Long = RubyVMNative.createInterpreter(
                    applicationPath,
                    location.rubyBaseDirectory,
                    location.nativeLibsLocation,
                    logListener
                )

              init {
                if (actualPtr == 0L) {
                        throw RuntimeException("Failed to create RubyInterpreter")
                    }
                }

                override fun getNativePtr(): Long = actualPtr

                // Forward LogListener methods to the provided listener
                override fun accept(lineMessage: String) {
                    logListener.accept(lineMessage)
                }

                override fun onLogError(errorMessage: String) {
                    logListener.onLogError(errorMessage)
                }
            }
        }
    }

    protected open fun getNativePtr(): Long = nativePtr

    fun enqueue(script: RubyScript, onComplete: CompletionCallback? = null) {
        RubyVMNative.enqueueScript(getNativePtr(), script.getNativePtr(), onComplete)
    }

    private fun destroy() {
        val ptr = getNativePtr()
        if (ptr != 0L) {
            RubyVMNative.destroyInterpreter(ptr)
        }
    }

    override fun close() = destroy()

    // Abstract LogListener methods - must be implemented by subclass
    abstract override fun accept(lineMessage: String)
    abstract override fun onLogError(errorMessage: String)
}
