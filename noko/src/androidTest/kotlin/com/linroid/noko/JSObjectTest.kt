package com.linroid.noko

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.linroid.noko.types.JSNumber
import com.linroid.noko.types.JSObject
import com.linroid.noko.types.JSString
import com.linroid.noko.types.JSValue
import com.linroid.noko.observable.PropertiesObserver
import com.nhaarman.mockitokotlin2.any
import com.nhaarman.mockitokotlin2.eq
import com.nhaarman.mockitokotlin2.mock
import com.nhaarman.mockitokotlin2.verify
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.internal.verification.AtLeast

@RunWith(AndroidJUnit4::class)
class JSObjectTest : nokoTest() {

  private lateinit var obj: JSObject

  @Before
  fun setup() = runInNode {
    obj = JSObject(context)
  }

  @Test
  fun setterAndGetter() = runInNode {
    obj.set("intValue", 1)
    val intValue = obj.get<JSValue>("intValue")
    assertInstance(intValue, JSNumber::class)
    assertEquals(1, intValue.toNumber().toInt())

    obj.set("floatValue", 1.0f)
    val floatValue = obj.get<JSValue>("floatValue")
    assertInstance(floatValue, JSNumber::class)
    assertEquals(1.0f, intValue.toNumber().toFloat())

    obj.set("doubleValue", 1.0)
    val doubleValue = obj.get<JSValue>("doubleValue")
    assertInstance(doubleValue, JSNumber::class)
    assertEquals(1.0, doubleValue.toNumber())

    obj.set("stringValue", "string")
    obj.set("stringValue", "string2")
    val stringValue = obj.get<JSValue>("stringValue")
    assertInstance(stringValue, JSString::class)
    assertEquals("string2", stringValue.toString())

    assertInstance(obj.get("floatValue"), JSNumber::class)
    assertInstance(obj.get("doubleValue"), JSNumber::class)
    assertInstance(obj.get("stringValue"), JSString::class)
  }

  @Test
  fun deleteAndHas() = runInNode {
    obj.set("foo", 1)
    assert(obj.has("foo"))
    assert(!obj.has("bar"))
    obj.delete("foo")
    assert(!obj.has("foo"))
  }

  @Test
  fun keys() = runInNode {
    obj.set("foo", 1)
    obj.set("bar", 1)
    assertArrayEquals(arrayOf("foo", "bar"), obj.keys())
  }

  @Test
  fun watchProperties() = runInNode {
    println("watchProperties")
    val observer = mock<PropertiesObserver>()
    obj.watch(observer, "name", "age")

    obj.set("name", "Foo")
    obj.set("name", "Bar")
    verify(observer, AtLeast(2)).onPropertyChanged(eq("name"), any<JSString>())

    obj.set("age", 18)
    verify(observer).onPropertyChanged(eq("age"), any<JSNumber>())
  }
}
