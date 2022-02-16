package com.linroid.noko

expect object Platform {
  fun isDebuggerConnected(): Boolean
  fun loadLibrary()
}