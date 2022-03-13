package com.linroid.noko

import kotlinx.coroutines.*

fun <T> runNodeTest(action: suspend Node.() -> T): Unit = runBlocking {
  val node = Node(null, keepAlive = true)
  launch {
    node.stdio.output().collect { println(it) }
  }
  launch {
    node.stdio.error().collect { System.err.println(it) }
  }
  node.start("-e", "process")
  withTimeout(3_000) {
    node.awaitStarted()
  }
  withContext(node.coroutineDispatcher) {
    node.action()
  }
  node.exit(0)
  withTimeout(3_000) {
    node.awaitStopped()
  }
}