package com.linroid.knode.js

import android.net.Uri
import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonSyntaxException
import com.linroid.knode.KNode
import java.io.Closeable
import java.lang.annotation.Native

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSValue(context: JSContext? = null, @Native protected val reference: Long) : Closeable {

    @Suppress("LeakingThis")
    lateinit var context: JSContext

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

    open fun toBoolean(): Boolean {
        return toNumber() != 0
    }

    fun typeOf(): String {
        return nativeTypeOf()
    }

    fun hasValue(): Boolean {
        return this !is JSNull && this !is JSUndefined
    }

    fun isPromise(): Boolean {
        return false
    }

    @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
    fun <T> toType(type: Class<T>): T? {
        if (JSValue::class.java != type && !this.hasValue()) {
            return null
        }
        val result = when {
            type == String::class.java -> this.toString()
            type == Any::class.java -> this
            type == Int::class.java -> toNumber().toInt()
            type == Boolean::class.java -> toBoolean()
            type == Long::class.java -> toNumber().toLong()
            type == Float::class.java -> toNumber().toFloat()
            type == Double::class.java -> toNumber().toDouble()
            type == Uri::class.java -> try {
                Uri.parse(toString())
            } catch (error: Exception) {
                null
            }
            type == JsonObject::class.java ||
                    type == JsonElement::class.java ||
                    type == JsonArray::class.java -> KNode.gson.fromJson(toJson(), JsonElement::class.java)
            JSValue::class.java.isAssignableFrom(type) -> this
            type.isArray -> {
                check(this is JSArray) { "$this is not an JSArray" }
                this.map { it.toType(type.componentType as Class<out Any>) }.toTypedArray()
            }
            else -> {
                val json = toJson()
                try {
                    KNode.gson.fromJson(json, type)
                } catch (error: JsonSyntaxException) {
                    throw IllegalArgumentException("Illegal data: $json, required an object")
                }
            }
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

    fun sameReference(other: JSValue): Boolean {
        return other.reference == reference
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
        private const val TAG = "JSValue"

        fun from(context: JSContext, value: Any?): JSValue {
            return when (value) {
                null -> context.sharedNull
                is JSValue -> value
                is Boolean -> if (value) context.sharedTrue else context.sharedFalse
                is String -> JSString(context, value)
                is Number -> JSNumber(context, value)
                is Iterator<*> -> JSArray(context, value)
                is List<*> -> JSArray(context, value.iterator())
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
