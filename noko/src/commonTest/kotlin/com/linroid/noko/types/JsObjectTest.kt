package com.linroid.noko.types

import com.linroid.noko.WithNode
import com.linroid.noko.observable.PropertiesObserver
import io.mockk.mockk
import io.mockk.verify
import kotlin.test.*

class JsObjectTest : WithNode() {

  private lateinit var obj: JsObject

  @BeforeTest
  override fun setUp() {
    super.setUp()
    joinNode {
      obj = JsObject(node)
    }
  }

  @Test
  fun setterAndGetter(): Unit = joinNode {
    obj.set("intValue", 1)
    val intValue = obj.get<Int>("intValue")
    assertEquals(1, intValue)

    obj.set("floatValue", 1.1f)
    val floatValue = obj.get<Float>("floatValue")
    assertEquals(1.1f, floatValue)

    obj.set("doubleValue", 1.1)
    val doubleValue = obj.get<Double>("doubleValue")
    assertEquals(1.1, doubleValue)

    obj.set("stringValue", "string")
    obj.set("stringValue", "string2")
    val stringValue = obj.get<String>("stringValue")
    assertEquals("string2", stringValue)

    assertIs<Double>(obj.get("floatValue"))
    assertIs<Double>(obj.get("doubleValue"))
    assertIs<String>(obj.get("stringValue"))
  }

  @Test
  fun deleteAndHas(): Unit = joinNode {
    obj.set("foo", 1)
    assertTrue(obj.has("foo"))
    assertTrue(!obj.has("bar"))
    obj.delete("foo")
    assertTrue(!obj.has("foo"))
  }

  @Test
  fun keys(): Unit = joinNode {
    obj.set("foo", 1)
    obj.set("bar", 1)
    assertContentEquals(arrayOf("foo", "bar"), obj.keys())
  }

  @Test
  fun watchProperties(): Unit = joinNode {
    val observer = mockk<PropertiesObserver>(relaxed = true)
    obj.watch(observer, "name", "age")

    obj.set("name", "Foo")
    obj.set("name", "Bar")
    verify(atLeast = 2) {
      observer.onPropertyChanged(eq("name"), any())
    }

    obj.set("age", 18)
    verify {
      observer.onPropertyChanged(eq("age"), any())
    }
  }
}
