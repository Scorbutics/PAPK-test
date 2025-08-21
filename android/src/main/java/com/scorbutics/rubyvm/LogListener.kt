package com.scorbutics.rubyvm

interface LogListener {
    fun accept(lineMessage: String)
    fun onLogError(errorMessage: String) // Changed from Exception to String for JNI compatibility
}
