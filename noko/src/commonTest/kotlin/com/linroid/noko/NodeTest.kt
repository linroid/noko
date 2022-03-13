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

class NodeTest : SetupNode() {

  @Test
  fun parseJson_object() = runNodeTest {
    val person = parseJson(
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
  fun parseJson_array() = runNodeTest {
    val numbers = parseJson(
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
  fun eval() = runNodeTest {
    assertIs<JsObject>(eval("process.versions"))
    assertNull(eval("console.log('Hello')"))
    val value = Random.nextInt()
    eval("global.testValue=$value;")
    assertEquals(value, global!!.get("testValue"))
  }

  @Test
  fun versions() = runNodeTest {
    val global = global!!
    val versions = global.get<JsObject>("process")!!.get<JsObject>("versions")
    val json = versions!!.toJson()
    assertNotNull(json)
    assertTrue(json.isNotEmpty())
  }

  @Test
  fun stdin_read() = runNodeTest {
    stdio.write("test")
    val result = coroutineScope {
      suspendCoroutine<Any> { cont ->
        launch {
          while (true) {
            val result = eval("process.stdin.read(4)")
            if (result != null) {
              cont.resume(result)
              break
            }
            yield()
          }
        }
      }
    }
    assertEquals("test", result.toString())
  }

  @Test
  fun stdin_on() = runNodeTest {
    stdio.write("test")
    withTimeout(3000) {
      val result = suspendCoroutine<Any?> { cont ->
        val stdin = global!!.get<JsObject>("process")!!.get<JsObject>("stdin")!!
        val callback = object : JsFunction(this@runNodeTest, "print") {
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
