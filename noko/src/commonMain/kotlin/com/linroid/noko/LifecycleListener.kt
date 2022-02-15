package com.linroid.noko

import com.linroid.noko.types.JSObject

/**
 * Listening lifecycle event from Node.js instance
 */
interface LifecycleListener {
  /**
   * Node.js is starting, do environment initialization work in this callback
   */
  fun onNodeBeforeStart(noko: Noko, global: JSObject) {}

  /**
   * The environment of Node.js is ready, this is called before executing the main script
   */
  fun onNodeStart(noko: Noko, global: JSObject) {}

  /**
   * Node.js is going to exit
   */
  fun onNodeBeforeExit(noko: Noko, global: JSObject) {}

  /**
   * Node.js is already exited, it's time to cleanup resources
   */
  fun onNodeExit(exitCode: Int) {}

  /**
   * Whoops :( something went wrong
   */
  fun onNodeError(error: JSException) {}
}
