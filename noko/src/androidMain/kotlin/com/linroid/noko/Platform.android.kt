package com.linroid.noko

import android.os.Debug
import okio.FileSystem

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM

  actual fun isDebuggerConnected(): Boolean {
    return Debug.isDebuggerConnected()
  }

  actual fun loadNative() {
    System.loadLibrary("noko")
  }
}