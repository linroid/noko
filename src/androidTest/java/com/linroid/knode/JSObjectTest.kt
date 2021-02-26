package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.linroid.knode.js.JSNumber
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSString
import com.linroid.knode.js.JSValue
import com.linroid.knode.observable.PropertyObserver
import com.nhaarman.mockitokotlin2.any
import com.nhaarman.mockitokotlin2.eq
import com.nhaarman.mockitokotlin2.mock
import com.nhaarman.mockitokotlin2.verify
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.internal.verification.AtLeast

/**
 * @author linroid
 * @since 2019/11/27
 */
@RunWith(AndroidJUnit4::class)
class JSObjectTest : KNodeTest() {

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
  fun watchProperties() = runInNode {
    println("watchProperties")
    val observer = mock<PropertyObserver>()
    obj.watch(observer, "name", "age")

    obj.set("name", "Foo")
    obj.set("name", "Bar")
    verify(observer, AtLeast(2)).onPropertyChanged(eq("name"), any<JSString>())

    obj.set("age", 18)
    verify(observer).onPropertyChanged(eq("age"), any<JSNumber>())
  }
}
