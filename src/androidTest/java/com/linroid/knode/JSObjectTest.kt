package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.linroid.knode.js.JSNumber
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSString
import com.linroid.knode.js.JSValue
import junit.framework.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * @author linroid
 * @since 2019/11/27
 */
@RunWith(AndroidJUnit4::class)
class JSObjectTest : KNodeTest() {

    private lateinit var obj: JSObject

    @Before
    fun setup() {
        obj = JSObject(context)
    }

    @Test
    fun setterAndGetter() {
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
        assertEquals(1.0, doubleValue.toNumber().toDouble())

        obj.set("stringValue", "string")
        obj.set("stringValue", "string2")
        val stringValue = obj.get<JSValue>("stringValue")
        assertInstance(doubleValue, JSString::class)
        assertEquals("string2", stringValue.toString())


        assertInstance(obj.get("floatValue"), JSNumber::class)
        assertInstance(obj.get("doubleValue"), JSNumber::class)
        assertInstance(obj.get("stringValue"), JSString::class)
    }
}