package com.linroid.noko

import okio.FileSystem
import java.io.File

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM
  private val isDebuggerConnected: Boolean = System.getProperty("java.vm.info", "").contains("sharing")

  actual fun isDebuggerConnected(): Boolean {
    return isDebuggerConnected
  }

  actual fun loadNativeLibraries() {
    // val libDir = File.createTempFile("noko", "noko")
    // libDir.mkdirs()
    // println(libDir)
    System.loadLibrary("noko")
  }
}
