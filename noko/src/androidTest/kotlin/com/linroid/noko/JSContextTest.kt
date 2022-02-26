package com.linroid.noko

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.linroid.noko.types.JsArray
import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsUndefined
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.random.Random

@RunWith(AndroidJUnit4::class)
class JSContextTest : NodeTest() {

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
    assertInstance(context.eval("process.versions"), JsObject::class)
    assertInstance(context.eval("console.log('Hello')"), JsUndefined::class)
    val value = Random.nextInt()
    context.eval("global.testValue=$value;")
    assertEquals(value, context.get<Int>("testValue"))
  }

  @Test
  fun versions() {
    val versions = context.get<JsObject>("process").get<JsObject>("versions")
    assertTrue(versions.toJson().isNotEmpty())
  }

  @Test
  fun parseJson() {
    val array = context.parseJson("[1]")
    assertInstance(array, JsArray::class)
    array as JsArray
    assertEquals(1, array[0].toNumber().toInt())

    val obj = context.parseJson("{ \"value\": 1}")
    assertInstance(array, JsObject::class)
    obj as JsObject
    assertEquals(1, obj.get<Int>("value"))
  }
}
