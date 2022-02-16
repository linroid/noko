package com.linroid.noko

expect class Thread

internal expect fun currentThread(): Thread

internal expect fun startThread(isDaemon: Boolean = false, name: String, block: () -> Unit): Thread


