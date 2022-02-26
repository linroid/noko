package com.linroid.noko

import com.linroid.noko.fs.FileSystem
import com.linroid.noko.fs.RealFileSystem
import com.linroid.noko.types.JsError
import com.linroid.noko.types.JsValue

/**
 * Create a new Node.js instance
 *
 * @param cwd The current work directory for node
 * @param output The standard output interface
 * @param fs
 * @param keepAlive If all the js code is executed completely, should we keep the node running
 * @param strictMode If true to do thread checking when doing operation on js objects
 */
expect class Node(
  cwd: String? = null,
  output: StdOutput,
  fs: FileSystem = RealFileSystem(),
  keepAlive: Boolean = false,
  strictMode: Boolean = true,
) {

  internal var ptr: Long

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
