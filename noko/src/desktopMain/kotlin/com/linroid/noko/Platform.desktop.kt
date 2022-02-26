package com.linroid.noko

import okio.FileSystem
import java.io.File

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM

  actual fun isDebuggerConnected(): Boolean {
    return false
  }

  actual fun loadNative() {
    val libDir = File.createTempFile("noko", "noko")
    libDir.mkdirs()
    println(libDir)
  }
}