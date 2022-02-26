package com.linroid.noko

import com.linroid.noko.types.JsObject
import kotlinx.coroutines.delay
import kotlin.test.Test

class NodeTest {
  @Test
  fun startNode() = runBlocking {
    val node = Node(null, object : StdOutput {
      override fun stdout(str: String) {
        println(str)
      }

      override fun stderr(str: String) {
        println(str)
      }

    }, keepAlive = true, strictMode = true)
    node.start("-p", "process.versions")
    node.addListener(object : LifecycleListener {
      override fun onNodeBeforeStart(node: Node, global: JsObject) {

      }
    })
    delay(3000)
  }
}