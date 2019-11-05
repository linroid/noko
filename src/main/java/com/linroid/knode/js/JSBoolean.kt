package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSBoolean : JSValue {
    private val data: Boolean

    @NativeConstructor
    private constructor(context: JSContext, reference: Long, data: Boolean) : super(context, reference) {
        this.data = data
    }

    constructor(context: JSContext, data: Boolean) : super(context, 0) {
        nativeNew(data)
        this.data = data
    }

    override fun toBoolean(): Boolean {
        return data
    }

    override fun toString(): String = data.toString()

    override fun toInt(): Int {
        return if (data) 1 else 0
    }

    override fun toDouble(): Double {
        return if (data) 1.0 else 0.0
    }

    private external fun nativeNew(data: Boolean)
}
