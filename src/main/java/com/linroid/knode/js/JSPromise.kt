package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019/11/1
 */
class JSPromise : JSObject {

    private var resolverPtr: Long = 0

    @NativeConstructor
    private constructor(context: JSContext, reference: Long) : super(context, reference)

    constructor(context: JSContext) : super(context, 0) {
        nativeNew()
    }

    fun reject(error: String) {
        nativeReject(JSError(context, error))
    }

    fun resolve(value: Any?) {
        nativeResolve(from(context, value))
    }

    fun then(callback: (JSValue) -> Unit): JSPromise {
        val then = JSFunction(context, "then") { _, argv ->
            check(argv.isNotEmpty()) { "then() should receive a JSValue argument" }
            val result: JSValue = argv[0]
            callback.invoke(result)
            return@JSFunction null
        }
        nativeThen(then)
        return this
    }

    fun catch(callback: (JSError) -> Unit): JSPromise {
        val then = JSFunction(context, "catch") { _, argv ->
            val result: JSValue = argv[0]
            check(result is JSError) { "catch() should receive an JSError parameter not ${result.javaClass.simpleName}(${result.typeOf()})}: ${result.toJson()}" }
            callback.invoke(result)
            return@JSFunction null
        }
        nativeCatch(then)
        return this
    }

    private external fun nativeNew()
    private external fun nativeReject(error: JSError)
    private external fun nativeResolve(value: JSValue)

    private external fun nativeThen(callback: JSFunction)
    private external fun nativeCatch(callback: JSFunction)
}
