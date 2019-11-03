package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-20
 */
class JSNumber : JSValue {
    private constructor(context: JSContext, reference: Long) : super(context, reference)
    constructor(context: JSContext, number: Number) : super(context, 0) {
        TODO("Not implemented")
    }
}
