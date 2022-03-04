package com.linroid.noko

import kotlinx.coroutines.withTimeout
import kotlin.test.AfterTest
import kotlin.test.BeforeTest

abstract class WithNode {

  protected lateinit var node: Node

  @BeforeTest
  fun setup(): Unit = runBlocking {
    node = Node(null, object : StdOutput {
      override fun stdout(str: String) {
        println(str)
      }

      override fun stderr(str: String) {
        println(str)
      }

    }, keepAlive = true, strictMode = true)

    node.start("-p", "process.versions")
    withTimeout(3000) {
      node.awaitStarted()
    }
  }

  @AfterTest
  fun tearDown() {
    node.exit(0)
  }

  protected fun <T> blockingInNode(action: () -> T) = runBlocking {
    node.await(action)
  }
}
