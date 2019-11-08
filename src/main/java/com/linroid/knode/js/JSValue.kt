package com.linroid.knode.js

import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.linroid.knode.KNode
import java.io.Closeable
import java.lang.annotation.Native

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSValue(context: JSContext? = null, @Native private val reference: Long) : Closeable {

    @Suppress("LeakingThis")
    protected lateinit var context: JSContext

    init {
        if (context != null) {
            this.context = context
        }
    }

    /** For native access runtime ptr */
    private fun runtime(): Long {
        return context.runtimePtr
    }

    override fun toString(): String {
        return nativeToString()
    }

    open fun toJson(): String {
        return nativeToJson()
    }

    open fun toNumber(): Number {
        return nativeToNumber()
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

    @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
    fun <T> toType(type: Class<T>): T? {
        if (this is JSNull || this is JSUndefined) {
            return null
        }
        val result = when {
            type == String::class.java -> this.toString()
            type == Any::class.java -> this
            type == Int::class.java -> toNumber().toInt()
            type == Long::class.java -> toNumber().toLong()
            type == Float::class.java -> toNumber().toFloat()
            type == Double::class.java -> toNumber().toDouble()
            type == JsonObject::class.java ||
                    type == JsonElement::class.java ||
                    type == JsonArray::class.java -> KNode.gson.toJsonTree(toJson())
            JSValue::class.java.isAssignableFrom(type) -> this
            type.isArray -> {
                check(this is JSArray) { "$this is not an JSArray" }
                this.map { it.toType(type.componentType as Class<out Any>) }.toTypedArray()
            }
            else -> throw TODO("Not support convert JSValue to $type class")
        }
        return result as T?
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
    private external fun nativeToNumber(): Double
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
                is Array<*> -> JSArray(context, value.iterator())
                is JsonElement -> {
                    context.parseJson(value.toString())
                }
                else -> {
                    context.parseJson(KNode.gson.toJson(value))
                }
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
