package com.linroid.noko

import kotlinx.coroutines.withTimeout
import kotlin.test.AfterTest
import kotlin.test.BeforeTest

abstract class WithNode {

  protected lateinit var node: Node

  @BeforeTest
  open fun setUp(): Unit = runBlocking {
    node = Node(null, object : StdOutput {
      override fun stdout(str: String) {
        println(str)
      }

      override fun stderr(str: String) {
        println(str)
      }

    }, keepAlive = true, strictMode = true)

    node.start("-p", "process.pid")
    withTimeout(3000) {
      node.awaitStarted()
    }
  }

  @AfterTest
  open fun tearDown() {
    node.exit(0)
  }

  protected fun <T> blockingInNode(action: () -> T) = runBlocking {
    node.await(action)
  }
}
