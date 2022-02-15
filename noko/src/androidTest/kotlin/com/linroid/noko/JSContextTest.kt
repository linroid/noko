package com.linroid.noko

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.linroid.noko.types.JSArray
import com.linroid.noko.types.JSObject
import com.linroid.noko.types.JSUndefined
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.random.Random

@RunWith(AndroidJUnit4::class)
class JSContextTest : nokoTest() {

  @Test
  fun sharedValue() {
    assertNotNull(context.sharedNull)
    assertNotNull(context.sharedUndefined)
    assertNotNull(context.sharedTrue)
    assertTrue(context.sharedTrue.get())
    assertFalse(context.sharedFalse.get())
  }

  @Test
  fun eval() {
    assertInstance(context.eval("process.versions"), JSObject::class)
    assertInstance(context.eval("console.log('Hello')"), JSUndefined::class)
    val value = Random.nextInt()
    context.eval("global.testValue=$value;")
    assertEquals(value, context.get<Int>("testValue"))
  }

  @Test
  fun versions() {
    val versions = context.get<JSObject>("process").get<JSObject>("versions")
    assertTrue(versions.toJson().isNotEmpty())
  }

  @Test
  fun parseJson() {
    val array = context.parseJson("[1]")
    assertInstance(array, JSArray::class)
    array as JSArray
    assertEquals(1, array[0].toNumber().toInt())

    val obj = context.parseJson("{ \"value\": 1}")
    assertInstance(array, JSObject::class)
    obj as JSObject
    assertEquals(1, obj.get<Int>("value"))
  }
}
