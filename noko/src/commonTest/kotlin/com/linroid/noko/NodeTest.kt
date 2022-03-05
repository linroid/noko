package com.linroid.noko

import com.linroid.noko.types.JsArray
import com.linroid.noko.types.JsNumber
import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsUndefined
import kotlin.random.Random
import kotlin.test.*

class NodeTest : WithNode() {

  @Test
  fun parseJson_object(): Unit = blockingInNode {
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
  fun parseJson_array() = blockingInNode {
    val numbers = node.parseJson(
      """
      [0, 1, 2, 3, 4]
    """.trimIndent()
    )
    assertIs<JsArray>(numbers)
    repeat(5) { index ->
      assertEquals((numbers[index] as JsNumber<*>).get().toInt(), index)
    }
  }

  @Test
  fun eval(): Unit = blockingInNode {
    assertIs<JsObject>(node.eval("process.versions"))
    assertIs<JsUndefined>(node.eval("console.log('Hello')"))
    val value = Random.nextInt()
    node.eval("global.testValue=$value;")
    assertEquals(value, node.global!!.get("testValue"))
  }

  @Test
  fun versions(): Unit = blockingInNode {
    val global = node.global!!
    val versions = global.get<JsObject>("process").get<JsObject>("versions")
    val json = versions.toJson()
    assertNotNull(json)
    assertTrue(json.isNotEmpty())
  }
}
