package com.scorbutics.rubyvm

import com.getcapacitor.JSObject
import com.getcapacitor.Plugin
import com.getcapacitor.PluginCall
import com.getcapacitor.PluginMethod
import com.getcapacitor.annotation.CapacitorPlugin
import java.util.IllegalFormatException
import java.util.Stack

@CapacitorPlugin(name = "RubyVM")
class RubyVMPlugin : Plugin() {

  private val logListener = object: LogListener {
    override fun accept(lineMessage: String) {
      notifyListeners("log", JSObject().put("message", lineMessage));
      /*m_currentLogs?.logs?.appendLine(lineMessage)
      runOnUiThread {
        expandableListAdapter.notifyDataSetChanged()
      }*/
    }

    override fun onLogError(errorMessage: String) {
      notifyListeners("logError", JSObject().put("error", errorMessage));
      //Toast.makeText(applicationContext, e, Toast.LENGTH_LONG).show()
    }
  }

  // Global interpreters storage
  private val interpreters = HashMap<Int, RubyInterpreter>()
  private val availableInterpreters = Stack<Int>()

  @PluginMethod
  fun create(call: PluginCall) {
    val nativeLibsLocation = context.applicationInfo.nativeLibraryDir
    val rubyBaseDirectory = context.filesDir.path
    val executionLocation = call.getString("executionLocation")
    val archiveLocation = call.getString("archiveLocation")

    System.out.println("Ruby Base Dir: " + rubyBaseDirectory)
    System.out.println("Native Libs Location:" + nativeLibsLocation)

    val location = ScriptCurrentLocation.create(rubyBaseDirectory, executionLocation, nativeLibsLocation, archiveLocation)
    val interpreter = RubyInterpreter.create(context.applicationInfo.dataDir, location, logListener)

    val index: Int = if (availableInterpreters.empty()) {
      interpreters.size
    } else {
      availableInterpreters.pop()
    }
    interpreters[index] = interpreter

    call.resolve(JSObject().put("interpreter", index))
  }

  @PluginMethod
  fun execute(call: PluginCall) {
    val interpreterIndex: Int = call.getInt("interpreter") ?: throw IllegalArgumentException("'execute' plugin call must contain an 'interpreter' value")
    val interpreter = interpreters[interpreterIndex] ?: throw IllegalArgumentException()

    try {
      RubyScript.fromContent("puts 'Hello World\n'")
        .use { script ->
          interpreter.enqueue(script) { result ->
            println("Script completed with result: $result")
            call.resolve(JSObject().put("result", result))
          }
        }
    } catch (error: Error) {
      call.reject(error.localizedMessage)
    }
  }

  @PluginMethod
  fun remove(call: PluginCall) {
    val interpreterIndex: Int = call.getInt("interpreter") ?: return
    if (interpreters.remove(interpreterIndex) != null) {
      availableInterpreters.push(interpreterIndex)
      call.resolve(JSObject().put("index", interpreterIndex))
    } else {
      call.resolve()
    }
  }

}
