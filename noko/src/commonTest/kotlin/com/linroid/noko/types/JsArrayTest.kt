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
    assertContentEquals(arrayListOf(0, 1, obj, 3), array)
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

  @Test
  fun removeAt(): Unit = joinNode {
    val array = JsArray(node, arrayListOf(0, 1, 2))
    val result = array.removeAt(1)
    assertIs<Int>(result)
    assertEquals(1, result)
    assertEquals(2, array.size)

    assertContentEquals(arrayListOf(0, 2), array)
  }

  @Test
  fun addAt(): Unit = joinNode {
    val array = JsArray(node, arrayListOf(0, 2, 3))
    array.add(1, 1)
    assertEquals(4, array.size)
    assertContentEquals(arrayListOf(0, 1, 2, 3), array)
  }

  @Test
  fun replaceAt(): Unit = joinNode {
    val array = JsArray(node, arrayListOf(0, 2, 3))
    array[1] = 1
    array[2] = 2
    assertEquals(3, array.size)
    assertContentEquals(arrayListOf(0, 1, 2), array)
  }

  @Test
  fun indexOf(): Unit = joinNode {
    val array = JsArray(node, arrayListOf(0, 2, 3))
    assertEquals(0, array.indexOf(0))
    assertEquals(1, array.indexOf(2))
    assertEquals(2, array.indexOf(3))
    assertEquals(-1, array.indexOf(-1))
  }
}