package com.linroid.noko

actual object Platform {
  actual fun isDebuggerConnected(): Boolean {
    return false
  }

  actual fun loadLibrary() {
    System.loadLibrary("noko")
  }
}