package com.scorbutics.rubyvm

object RubyVMNative {
    init {
        System.loadLibrary("jni")
    }

    external fun createInterpreter(
        appPath: String,
        rubyBaseDirectory: String,
        nativeLibsLocation: String,
        listener: LogListener
    ): Long

    external fun updateEnvLocations(currentDirectory: String, extraArg: String): Int

    external fun destroyInterpreter(interpreterPtr: Long)

    external fun createScript(content: String): Long

    external fun destroyScript(scriptPtr: Long)

    external fun enqueueScript(
        interpreterPtr: Long,
        scriptPtr: Long,
        completionCallback: CompletionCallback?
    )
}
