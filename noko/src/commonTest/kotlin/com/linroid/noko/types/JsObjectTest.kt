package com.linroid.noko.types

import com.linroid.noko.SetupNode
import com.linroid.noko.observable.PropertiesObserver
import com.linroid.noko.runNodeTest
import io.mockk.mockk
import io.mockk.verify
import kotlin.test.*

class JsObjectTest : SetupNode() {

  @Test
  fun setterAndGetter() = runNodeTest {
    val obj = JsObject(this)

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
  fun deleteAndHas() = runNodeTest {
    val obj = JsObject(this)

    obj.set("foo", 1)
    assertTrue(obj.has("foo"))
    assertTrue(!obj.has("bar"))
    obj.delete("foo")
    assertTrue(!obj.has("foo"))
  }

  @Test
  fun keys() = runNodeTest {
    val obj = JsObject(this)

    obj.set("foo", 1)
    obj.set("bar", 1)
    assertContentEquals(arrayOf("foo", "bar"), obj.keys())
  }

  @Test
  fun watchProperties() = runNodeTest {
    val obj = JsObject(this)

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
