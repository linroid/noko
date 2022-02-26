package com.linroid.noko.ref

import com.linroid.noko.types.JsValue
import java.lang.ref.ReferenceQueue

internal object ReferenceWatcher : Thread("js-object-watcher") {

  private const val TAG = "ReferenceWatcher"

  val queue = ReferenceQueue<JsValue>()

  override fun run() {
    while (true) {
      val ref = queue.remove()
      ref.clear()
    }
  }
}
