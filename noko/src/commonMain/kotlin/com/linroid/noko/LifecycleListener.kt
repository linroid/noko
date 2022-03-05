package com.linroid.noko

import com.linroid.noko.types.JsObject

/**
 * Listening lifecycle events from Node.js runtime
 */
interface LifecycleListener {
  /**
   * Node.js is starting, do environment initialization work in this callback
   */
  fun onAttach(node: Node, global: JsObject) {}

  /**
   * The runtime is ready, called before executing the main script
   */
  fun onStart(node: Node, global: JsObject) {}

  /**
   * Node.js is going to exit
   */
  fun onDetach(node: Node, global: JsObject) {}

  /**
   * Node.js is already exited, it's time to cleanup resources
   */
  fun onStop(code: Int) {}

  /**
   * Whoops :( something went wrong
   */
  fun onError(error: JsException) {}
}
