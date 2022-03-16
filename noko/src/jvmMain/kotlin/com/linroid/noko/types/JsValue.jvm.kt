package com.linroid.noko.types

import com.linroid.noko.*
import com.linroid.noko.annotation.ForNative
import com.linroid.noko.ref.JSValueReference

actual open class JsValue actual constructor(
  protected actual val node: Node,
  @JvmField
  internal actual val pointer: NativePointer,
) {
  init {
    if (pointer != 0L) {
      // The object is just a wrapper for js object
      JSValueReference(this, node.cleaner)
    }
  }

  @ForNative
  private fun nodePointer(): Long {
    return node.pointer
  }

  actual override fun toString(): String {
    if (!node.isInEventLoop() && Platform.isDebuggerConnected()) {
      return runBlocking(node.coroutineDispatcher) {
        try {
          nativeToString()
        } catch (error: JsException) {
          "Invalid JsValue"
        }
      }
    }
    if (!hasValue()) {
      return "null"
    }
    return nativeToString()
  }

  actual open fun toJson(): String? {
    return nativeToJson()
  }

  actual open fun toNumber(): Number {
    return nativeToNumber()
  }

  actual open fun toBoolean(): Boolean {
    return toNumber() != 0
  }

  actual fun typeOf(): String {
    if (pointer == NullNativePointer) {
      return "undefined"
    }
    return nativeTypeOf()
  }

  actual fun hasValue(): Boolean {
    return this !is JsUndefined && pointer != NullNativePointer
  }

  actual fun isPromise(): Boolean {
    return false
  }

  actual fun dispose() {
    nativeDispose()
  }

  actual override fun equals(other: Any?): Boolean {
    if (other is JsValue) {
      if (other.pointer == pointer) {
        return true
      }
      return nativeEquals(other)
    }
    return super.equals(other)
  }

  actual override fun hashCode(): Int {
    return pointer.hashCode()
  }

  private external fun nativeEquals(other: JsValue): Boolean
  private external fun nativeToJson(): String?
  private external fun nativeToString(): String
  private external fun nativeToNumber(): Double
  private external fun nativeTypeOf(): String
  private external fun nativeDispose()

}
