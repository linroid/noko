package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSBoolean @NativeConstructor private constructor(context: JSContext, private val data: Boolean) : JSPrimitive(context, 0) {

    fun get(): Boolean = data

    override fun toString(): String = data.toString()

    override fun toBoolean(): Boolean {
        return data
    }
}
