package com.linroid.noko

import okio.FileSystem

expect object Platform {
  val fileSystem: FileSystem
  fun isDebuggerConnected(): Boolean
  fun loadLibrary()
}