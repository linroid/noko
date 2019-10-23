package com.linroid.knode.js

import java.lang.reflect.Method

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSObject(context: JSContext?, reference: Long) : JSValue(context, reference) {
    fun has(key: String): Boolean {
        return nativeHas(key)
    }

    fun set(key: String, value: JSValue) {
        nativeSet(key, value)
    }

    fun get(key: String): JSValue {
        return nativeGet(key)
    }

    fun keys(): Array<String> {
        return nativeKeys()
    }

    fun registerMethod(obj: Any, methodName: String, jsFunctionName: String) {
        val method = obj.javaClass.getMethod(methodName)
        method.isAccessible = true
        nativeRegisterMethod(jsFunctionName, method)
    }

    private external fun nativeKeys(): Array<String>
    private external fun nativeRegisterMethod(name: String, method: Method)
    private external fun nativeHas(key: String): Boolean
    private external fun nativeGet(key: String): JSValue
    private external fun nativeSet(key: String, value: JSValue): Boolean
}
