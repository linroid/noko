package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSUndefined : JSPrimitive {
    @NativeConstructor
    private constructor (context: JSContext, reference: Long) : super(context, reference)

    constructor(context: JSContext) : super(context, 0) {
        nativeNew()
    }

    private external fun nativeNew()

    override fun toJson(): String {
        return ""
    }
}
