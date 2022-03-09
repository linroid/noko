package com.linroid.noko

import kotlinx.coroutines.*
import kotlin.test.AfterTest
import kotlin.test.BeforeTest

abstract class WithNode {

  protected lateinit var node: Node
  private var scope = CoroutineScope(Dispatchers.IO)

  @BeforeTest
  open fun setUp(): Unit = runBlocking {
    node = Node(null, keepAlive = true, strictMode = true)
    scope.launch {
      node.stdio.output().collect { println(it) }
    }
    node.start("-p", "process.pid")
    withTimeout(10_000) {
      node.awaitStarted()
    }
  }

  @AfterTest
  open fun tearDown() {
    runBlocking {
      val job = launch {
        node.awaitStopped()
      }
      node.exit(0)
      withTimeout(10_000) {
        job.join()
      }
    }
  }

  protected fun <T> joinNode(action: suspend () -> T) = runBlocking {
    withContext(node.coroutineDispatcher) {
      action()
    }
  }
}
