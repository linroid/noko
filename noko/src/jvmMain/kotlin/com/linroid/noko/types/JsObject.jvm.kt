package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer
import com.linroid.noko.observable.PropertiesObserver
import kotlin.reflect.KClass

actual open class JsObject : JsValue {

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer)

  actual constructor(node: Node) : super(node, NullNativePointer) {
    node.checkThread()
    nativeNew()
    addBinds()
  }

  private fun addBinds() {
    if (this::class === JsObject::class) {
      return
    }
    // TODO: implement by ksp
    // val className = this::class.simpleName
    // this::class.methods
    //   .filter { it.isAnnotationPresent(JSName::class.java) }
    //   .forEach { method ->
    //     method.isAccessible = true
    //     val bind = method.getAnnotation(JSName::class.java)!!
    //     val name = bind.name.ifEmpty { method.name }
    //     set(name, object : JsFunction(noko, name) {
    //       override fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue {
    //         val result = try {
    //           method.invoke(this@JsObject, *convertParameters(parameters, method.parameterTypes))
    //         } catch (error: InvocationTargetException) {
    //           noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.targetException.message}")
    //         } catch (error: Exception) {
    //           noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.message}")
    //         }
    //         return from(noko, result)
    //       }
    //     })
    //   }
  }

  private fun convertParameters(
    parameters: Array<out JsValue>,
    parameterTypes: Array<Class<*>>
  ): Array<Any?> {
    val argc = parameterTypes.size
    return Array(argc) { i ->
      val value = parameters[i]
      val type = parameterTypes[i]
      value.toType(type)
    }
  }

  actual fun has(key: String): Boolean {
    return nativeHas(key)
  }

  actual fun set(key: String, value: Any?) {
    nativeSet(key, from(node, value))
  }

  actual inline fun <reified T : Any> get(key: String): T {
    return get(key, T::class.java)
      ?: throw IllegalStateException("get $key from $this shouldn't return null")
  }

  actual inline fun <reified T : Any> opt(key: String): T? {
    return get(key, T::class.java)
  }

  fun <T : Any> get(key: String, clazz: Class<T>): T? {
    node.checkThread()
    val value = nativeGet(key)
    return value.toType(clazz)
  }

  actual fun delete(key: String) {
    nativeDelete(key)
  }

  actual fun keys(): Array<String> {
    return nativeKeys()
  }

  actual fun watch(observer: PropertiesObserver, vararg properties: String) {
    nativeWatch(properties, observer)
  }

  private external fun nativeGet(key: String): JsValue
  private external fun nativeKeys(): Array<String>
  private external fun nativeHas(key: String): Boolean
  private external fun nativeSet(key: String, value: JsValue?)
  private external fun nativeDelete(key: String)
  private external fun nativeNew()
  private external fun nativeWatch(properties: Array<out String>, observer: PropertiesObserver)
}
