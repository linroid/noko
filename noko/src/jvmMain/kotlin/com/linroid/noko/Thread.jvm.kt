package com.linroid.noko

import kotlin.concurrent.thread

internal actual typealias Thread = java.lang.Thread

internal actual fun currentThread(): Thread {
  return Thread.currentThread()
}

internal actual fun startThread(isDaemon: Boolean, name: String, block: () -> Unit): Thread {
  return thread(name = name, block = block)
}


