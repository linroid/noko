package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Test
import org.junit.runner.RunWith
import java.lang.ref.PhantomReference
import java.lang.ref.ReferenceQueue
import java.util.*

/**
 * Created by linroid on 1/29/21.
 */
@RunWith(AndroidJUnit4::class)
class ReferenceTest {
  @Test(timeout = 10000)
  fun phantom() {
    val queue = ReferenceQueue<Any>()
    val refs: MutableList<Any> = ArrayList()
    repeat(100) {
      refs.add(PhantomReference(Any(), queue))
    }
    repeat(100) {
      println("remove")
      Runtime.getRuntime().gc()
      queue.remove()
    }
  }
}