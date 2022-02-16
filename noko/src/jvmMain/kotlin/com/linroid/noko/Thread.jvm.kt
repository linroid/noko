package com.linroid.noko

import kotlinx.coroutines.CoroutineScope
import kotlin.concurrent.thread
import kotlin.coroutines.CoroutineContext

internal actual typealias Thread = java.lang.Thread

internal actual fun currentThread(): Thread {
  return Thread.currentThread()
}

internal actual fun startThread(isDaemon: Boolean, name: String, block: () -> Unit): Thread {
  return thread(name = name, block = block)
}

actual fun <T> runBlocking(
  context: CoroutineContext,
  block: suspend CoroutineScope.() -> T
) = kotlinx.coroutines.runBlocking(context, block)