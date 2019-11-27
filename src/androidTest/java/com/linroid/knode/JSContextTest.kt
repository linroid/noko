package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.linroid.knode.js.JSArray
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSUndefined
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.random.Random
import kotlin.reflect.KClass

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
    fun versionsAssets() {
        val appContext = InstrumentationRegistry.getInstrumentation().targetContext
        val text = appContext.assets.open("knode_versions.json").use { input ->
            input.reader().use { reader ->
                reader.readText()
            }
        }
        assertTrue(text.isNotEmpty())
        val json = JSONObject(text)
        val versions = context.get<JSObject>("process").get<JSObject>("versions")
        json.keys().forEach { key ->
            assertEquals(json.get(key), versions.get<String>(key))
        }
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

    private fun assertInstance(obj: Any, clazz: KClass<*>) {
        assertTrue("${obj.javaClass} is not an instance of $clazz ", clazz.isInstance(obj))
    }
}
