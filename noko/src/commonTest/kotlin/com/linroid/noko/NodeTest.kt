package com.linroid.noko

import com.linroid.noko.types.JsObject
import kotlinx.coroutines.withTimeout
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertIs

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

  @Test
  fun parseJsonObject(): Unit = runBlocking {
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
    node.await {
      val person = node.parseJson(
        """
      {
        "name": "Foo",
        "age": 18,
        "is_adult": true
      }
    """.trimIndent()
      )
      assertIs<JsObject>(person)
      assertEquals("Foo", person.get("name"))
      assertEquals(18, person.get("age"))
      assertEquals(true, person.get("is_adult"))
    }
  }
}
