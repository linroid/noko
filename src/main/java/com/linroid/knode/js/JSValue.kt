package com.linroid.knode.js

import com.google.gson.JsonElement
import java.io.Closeable
import java.lang.annotation.Native

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSValue(
    context: JSContext?,
    @Native protected val reference: Long
) : Closeable {

    @Suppress("LeakingThis")
    val context: JSContext = context ?: this as JSContext

    override fun toString(): String {
        return nativeToString()
    }

    open fun toJson(): String {
        return nativeToJson()
    }

    open fun toInt(): Int {
        return 0
    }

    open fun toDouble(): Double {
        return 0.0
    }

    open fun toBoolean(): Boolean {
        return true
    }

    fun typeOf(): String {
        return nativeTypeOf()
    }

    fun empty(): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    fun isPromise(): Boolean {
        return false
    }

    override fun close() {
        nativeDispose()
    }

    @Throws(Throwable::class)
    protected open fun finalize() {
//        if (BuildConfig.DEBUG) {
//            check(reference != 0L) { "No v8 object referenced" }
//        }
//        if (reference != 0L) {
//            Log.i("JSValue", "nativeDispose: $reference")
//            nativeDispose()
//        }
    }

    override fun equals(other: Any?): Boolean {
        if (other is JSValue) {
            if (other.reference == reference) {
                return true
            }
            return nativeEquals(other)
        }
        return super.equals(other)
    }

    override fun hashCode(): Int {
        return reference.hashCode()
    }

    private external fun nativeEquals(other: JSValue): Boolean
    private external fun nativeToJson(): String
    private external fun nativeToString(): String
    private external fun nativeTypeOf(): String
    private external fun nativeDispose()

    companion object {
        fun from(context: JSContext, value: Any?): JSValue {
            return when (value) {
                null -> JSNull(context)
                is JSValue -> value
                is String -> JSString(context, value)
                is Number -> JSNumber(context, value)
                is Iterator<*> -> JSArray(context, value)
                is JsonElement -> {
                    // from(context, value)
                    context.parseJson(value.toString())
                }
                else -> throw IllegalStateException("Not support ${value.javaClass}")
            }
        }

        // fun from(context: JSContext, value: JsonElement): JSValue {
        //     return when {
        //         value.isJsonNull -> JSNull(context)
        //         value.isJsonObject -> JSObject(context, value.asJsonObject)
        //         value.isJsonArray -> JSArray(context, value as Iterator<*>)
        //         value.isJsonPrimitive -> {
        //             value as JsonPrimitive
        //             when {
        //                 value.isBoolean -> JSBoolean(context, value.asBoolean)
        //                 value.isNumber -> JSNumber(context, value.asNumber)
        //                 value.isString -> JSString(context, value.asString)
        //                 else -> throw IllegalStateException("Not support JsonPrimitive: ${value.javaClass}")
        //             }
        //         }
        //         else -> throw IllegalStateException("Not support Json type: ${value.javaClass}")
        //     }
        // }
    }
}
