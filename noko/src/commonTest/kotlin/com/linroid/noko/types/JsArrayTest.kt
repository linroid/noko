package com.linroid.noko.types

import com.linroid.noko.WithNode
import kotlin.test.*

class JsArrayTest : WithNode() {

  @Test
  fun `size and get`(): Unit = joinNode {
    //language=JSON
    val array = node.parseJson("[1, 1.1, \"string\", {\"name\": \"foo\"}, [1, 2, 3, 4]]")
    assertIs<JsArray>(array)
    assertEquals(array.size, 5)
    assertEquals(array[0], 1)
    assertEquals(array[1], 1.1)
    assertEquals(array[2], "string")
    assertIs<JsObject>(array[3])
    assertIs<JsArray>(array[4])
  }

  @Test
  fun `add and get`(): Unit = joinNode {
    val obj = JsObject(node)
    val array = JsArray(node, arrayListOf(0, 1, obj))
    array.add(3)
    assertEquals(array[0], 0)
    assertEquals(array[1], 1)
    assertEquals(array[2], obj)
    assertEquals(array[3], 3)
  }

  @Test
  fun clear(): Unit = joinNode {
    val array = JsArray(node, arrayListOf(0, 1, 3))
    assertEquals(3, array.size)
    assertTrue { array.isNotEmpty() }
    array.clear()
    assertNull(array[0])
    assertEquals(0, array.size)
    assertTrue { array.isEmpty() }

    array.add(4)
    assertEquals(1, array.size)
  }
}