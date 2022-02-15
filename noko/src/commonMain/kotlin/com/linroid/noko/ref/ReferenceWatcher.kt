package com.linroid.noko.ref

import com.linroid.noko.types.JSValue
import java.lang.ref.ReferenceQueue

internal object ReferenceWatcher : Thread("js-object-watcher") {

  private const val TAG = "ReferenceWatcher"

  val queue = ReferenceQueue<JSValue>()

  override fun run() {
    while (true) {
      val ref = queue.remove()
      ref.clear()
    }
  }
}
