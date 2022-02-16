package com.linroid.noko

import android.os.Debug

actual object Platform {
  actual fun isDebuggerConnected(): Boolean {
    return Debug.isDebuggerConnected()
  }
}