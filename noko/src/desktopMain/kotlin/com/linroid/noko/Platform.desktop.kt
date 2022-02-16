package com.linroid.noko

import okio.FileSystem

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM

  actual fun isDebuggerConnected(): Boolean {
    return false
  }

  actual fun loadLibrary() {
    System.loadLibrary("noko")
  }
}