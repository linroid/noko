package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.linroid.knode.js.JSArray
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSUndefined
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.random.Random

/**
 * @author linroid
 * @since 2019/11/27
 */
@RunWith(AndroidJUnit4::class)
class JSContextTest : KNodeTest() {

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
    val appContext = InstrumentationRegistry.getInstrumentation().targetContext
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
