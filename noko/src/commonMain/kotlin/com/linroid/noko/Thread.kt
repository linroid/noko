package com.linroid.noko

import kotlinx.coroutines.CoroutineScope
import kotlin.coroutines.CoroutineContext
import kotlin.coroutines.EmptyCoroutineContext

expect class Thread

internal expect fun currentThread(): Thread

internal expect fun startThread(isDaemon: Boolean = false, name: String, block: () -> Unit): Thread

expect fun <T> runBlocking(
  context: CoroutineContext = EmptyCoroutineContext,
  block: suspend CoroutineScope.() -> T
): T