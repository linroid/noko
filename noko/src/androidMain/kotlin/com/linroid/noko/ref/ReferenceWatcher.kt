package com.linroid.noko.ref

import android.util.Log
import com.linroid.noko.type.JSValue
import java.lang.ref.ReferenceQueue

internal object ReferenceWatcher : Thread("js-object-watcher") {

  private const val TAG = "ReferenceWatcher"

  val queue = ReferenceQueue<JSValue>()

  override fun run() {
    Log.i(TAG, "start js object reference watcher...")
    while (true) {
      val ref = queue.remove()
      ref.clear()
    }
  }
}
