package com.linroid.noko

import kotlinx.coroutines.*
import kotlin.test.AfterTest
import kotlin.test.BeforeTest

abstract class WithNode : SetupNode() {

  protected lateinit var node: Node
  private var scope = CoroutineScope(Dispatchers.IO)

  @BeforeTest
  open fun setUp() {
    scope = CoroutineScope(Dispatchers.IO)
    node = Node(null, keepAlive = true)
    scope.launch {
      node.stdio.output().collect { println(it) }
    }
    scope.launch {
      node.stdio.error().collect { System.err.println(it) }
    }
    node.start("-p", "process.pid")
    runBlocking {
      withTimeout(3_000) {
        node.awaitStarted()
      }
    }
  }

  @AfterTest
  open fun tearDown() {
    runBlocking {
      node.exit(0)
      withTimeout(3_000) {
        node.awaitStopped()
      }
    }
  }

  protected fun <T> joinNode(action: suspend () -> T) = runBlocking {
    withContext(node.coroutineDispatcher) {
      action()
    }
  }
}
