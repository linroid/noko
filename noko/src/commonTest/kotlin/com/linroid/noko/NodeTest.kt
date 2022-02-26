package com.linroid.noko

import kotlinx.coroutines.withTimeout
import kotlin.test.Test

class NodeTest {

  @Test
  fun startNode(): Unit = runBlocking {
    val node = Node(null, object : StdOutput {
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
}
