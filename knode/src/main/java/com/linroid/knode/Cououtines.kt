package com.linroid.knode

import com.linroid.knode.js.JSPromise
import com.linroid.knode.js.JSValue
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

/**
 * @author linroid
 * @since 2020/3/8
 */
suspend fun JSPromise.await(): JSValue {
  return suspendCancellableCoroutine { continuation ->
    if (!context.node.post {
        then {
          continuation.resume(it)
        }.catch {
          continuation.resumeWithException(JSException(it))
        }
      }) {
      continuation.cancel()
    }
  }
}

suspend fun JSValue.awaitIfPromise(): JSValue {
  if (this is JSPromise) {
    return this.await()
  }
  return this
}

suspend fun <T, V : JSValue> V.awaitWith(action: V.() -> T): T {
  return suspendCancellableCoroutine { continuation ->
    val success = context.node.post {
      try {
        continuation.resume(action())
      } catch (error: Exception) {
        continuation.cancel(error)
      }
    }
    if (!success) {
      continuation.cancel()
    }
  }
}

suspend fun <T> KNode.await(action: () -> T): T {
  return suspendCancellableCoroutine { continuation ->
    val success = post {
      try {
        continuation.resume(action())
      } catch (error: Exception) {
        continuation.cancel(error)
      }
    }
    if (!success) {
      continuation.cancel()
    }
  }
}