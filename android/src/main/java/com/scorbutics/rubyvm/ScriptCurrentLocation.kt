package com.scorbutics.rubyvm

class ScriptCurrentLocation private constructor(
  val rubyBaseDirectory: String,
  val executionLocation: String?,
  val nativeLibsLocation: String,
  val archiveLocation: String?
) {

    companion object {
        fun create(
            rubyBaseDirectory: String,
            executionLocation: String?,
            nativeLibsLocation: String,
            archiveLocation: String?
        ): ScriptCurrentLocation {
            return ScriptCurrentLocation(rubyBaseDirectory, executionLocation, nativeLibsLocation, archiveLocation)
        }
    }
}
