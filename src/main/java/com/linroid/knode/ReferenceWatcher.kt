package com.linroid.knode

import android.util.Log
import com.linroid.knode.js.JSValue
import java.lang.ref.ReferenceQueue

/**
 * Created by linroid on 1/27/21.
 */
internal object ReferenceWatcher : Thread("js-object-reference-watcher") {

  private const val TAG = "ReferenceWatcher"

  val queue = ReferenceQueue<JSValue>()

  override fun run() {
    Log.i(TAG, "start reference watcher...")
    while (true) {
      val ref = queue.remove()
      ref.clear()
    }
  }
}