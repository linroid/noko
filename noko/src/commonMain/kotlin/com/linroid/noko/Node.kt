package com.linroid.noko

import com.linroid.noko.fs.FileSystem
import com.linroid.noko.fs.RealFileSystem
import com.linroid.noko.types.JsError
import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsValue
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resumeWithException

/**
 * Create a new Node.js instance
 *
 * @param cwd The current work directory for node
 * @param output The standard output interface
 * @param fs The file system for Node.js
 * @param keepAlive true if event loop should keep running after the main script is executed completely,
 * @param strictMode true to do thread checking when doing operation on js objects
 */
expect class Node(
  cwd: String? = null,
  output: StdOutput,
  fs: FileSystem = RealFileSystem(),
  keepAlive: Boolean = false,
  strictMode: Boolean = true,
) {

  internal var pointer: NativePointer

  internal val cleaner: (Long) -> Unit

  /**
   * Start node instance with arguments
   */
  fun start(vararg args: String)

  /**
   * Add a listener to listen the state of node instance
   */
  fun addListener(listener: LifecycleListener): Boolean

  /**
   * Removes a listener from this node instance
   */
  fun removeListener(listener: LifecycleListener): Boolean

  fun addEnv(key: String, value: String)

  fun addVersion(key: String, value: String)

  suspend fun <T> await(action: () -> T): T

  /**
   * Instructs the VM to halt execution as quickly as possible
   * @param code The exit code
   */
  fun exit(code: Int)

  fun post(action: () -> Unit): Boolean

  internal fun checkThread()

  internal fun isInNodeThread(): Boolean

  @Throws(JSException::class)
  fun eval(
    code: String,
    source: String = "",
    line: Int = 0,
  ): JsValue

  @Throws(JSException::class)
  fun parseJson(json: String): JsValue

  fun throwError(message: String): JsError

  @Deprecated("Not working")
  @Throws(JSException::class)
  fun require(path: String): JsValue
}

suspend fun Node.awaitStarted(): JsObject = suspendCancellableCoroutine { cont ->
  val listener = object : LifecycleListener {
    override fun onStart(node: Node, global: JsObject) {
      cont.resumeWith(Result.success(global))
      removeListener(this)
    }

    override fun onError(error: JSException) {
      cont.resumeWithException(error)
      removeListener(this)
    }
  }
  addListener(listener)
}

