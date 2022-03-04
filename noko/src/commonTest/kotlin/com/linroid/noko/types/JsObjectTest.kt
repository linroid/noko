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
    blockingInNode {
      obj = JsObject(node)
    }
  }

  @Test
  fun setterAndGetter(): Unit = blockingInNode {
    obj.set("intValue", 1)
    val intValue = obj.get<JsValue>("intValue")
    assertIs<JsNumber<*>>(intValue)
    assertEquals(1, intValue.toNumber().toInt())

    obj.set("floatValue", 1.0f)
    val floatValue = obj.get<JsValue>("floatValue")
    assertIs<JsNumber<*>>(floatValue)
    assertEquals(1.0f, intValue.toNumber().toFloat())

    obj.set("doubleValue", 1.0)
    val doubleValue = obj.get<JsValue>("doubleValue")
    assertIs<JsNumber<*>>(doubleValue)
    assertEquals(1.0, doubleValue.toNumber())

    obj.set("stringValue", "string")
    obj.set("stringValue", "string2")
    val stringValue = obj.get<JsValue>("stringValue")
    assertIs<JsString>(stringValue)
    assertEquals("string2", stringValue.toString())

    assertIs<JsNumber<*>>(obj.get("floatValue"))
    assertIs<JsNumber<*>>(obj.get("doubleValue"))
    assertIs<JsString>(obj.get("stringValue"))
  }

  @Test
  fun deleteAndHas(): Unit = blockingInNode {
    obj.set("foo", 1)
    assertTrue(obj.has("foo"))
    assertTrue(!obj.has("bar"))
    obj.delete("foo")
    assertTrue(!obj.has("foo"))
  }

  @Test
  fun keys(): Unit = blockingInNode {
    obj.set("foo", 1)
    obj.set("bar", 1)
    assertContentEquals(arrayOf("foo", "bar"), obj.keys())
  }

  @Test
  fun watchProperties(): Unit = blockingInNode {
    println("watchProperties")
    val observer = mockk<PropertiesObserver>(relaxed = true)
    obj.watch(observer, "name", "age")

    obj.set("name", "Foo")
    obj.set("name", "Bar")
    verify(atLeast = 2) {
      observer.onPropertyChanged(eq("name"), any<JsString>())
    }

    obj.set("age", 18)
    verify {
      observer.onPropertyChanged(eq("age"), any<JsString>())
    }
  }
}
