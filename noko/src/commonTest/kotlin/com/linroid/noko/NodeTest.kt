package com.linroid.noko

import com.linroid.noko.types.JsArray
import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsFunction
import com.linroid.noko.types.JsValue
import kotlinx.coroutines.*
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine
import kotlin.random.Random
import kotlin.test.*

class NodeTest : WithNode() {

  @Test
  fun parseJson_object(): Unit = joinNode {
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

  @Test
  fun parseJson_array() = joinNode {
    val numbers = node.parseJson(
      """
      [0, 1, 2, 3, 4]
    """.trimIndent()
    )
    assertIs<JsArray>(numbers)
    repeat(5) { index ->
      assertIs<Number>(numbers[index])
    }
  }

  @Test
  fun eval(): Unit = joinNode {
    assertIs<JsObject>(node.eval("process.versions"))
    assertNull(node.eval("console.log('Hello')"))
    val value = Random.nextInt()
    node.eval("global.testValue=$value;")
    assertEquals(value, node.global!!.get("testValue"))
  }

  @Test
  fun versions(): Unit = joinNode {
    val global = node.global!!
    val versions = global.get<JsObject>("process")!!.get<JsObject>("versions")
    val json = versions!!.toJson()
    assertNotNull(json)
    assertTrue(json.isNotEmpty())
  }

  @Test
  fun stdin_read(): Unit = joinNode {
    node.stdio.write("test")
    withContext(Dispatchers.IO) {
      yield()
    }
    assertEquals("test", node.eval("process.stdin.read(4)").toString())
  }

  @Test
  fun stdin_on(): Unit = joinNode {
    node.stdio.write("test")
    withTimeout(3000) {
      val result = suspendCoroutine<Any?> { cont ->
        val stdin = node.global!!.get<JsObject>("process")!!.get<JsObject>("stdin")!!
        val callback = object : JsFunction(node, "print") {
          override fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any? {
            cont.resume(parameters[0])
            return null
          }
        }
        val onFunc = stdin.get<JsFunction>("on")!!
        onFunc.call(stdin, "data", callback)
      }
      assertEquals("test", result.toString())
    }
  }
}
