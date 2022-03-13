package com.linroid.noko

import okio.FileSystem

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM
  private val isDebuggerConnected: Boolean = try {
    // System.getProperty("java.vm.info", "").contains("sharing")
    Class.forName("org.junit.Test")
    true
  } catch (e: ClassNotFoundException) {
    false
  }

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
