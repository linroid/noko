package com.linroid.noko.types

import com.linroid.noko.SetupNode
import com.linroid.noko.runNodeTest
import kotlinx.coroutines.delay
import kotlinx.coroutines.withTimeout
import kotlinx.coroutines.yield
import java.lang.ref.WeakReference
import kotlin.test.*

class JsFunctionTest : SetupNode() {
  @Test
  fun shouldBeReleasedAfterDeleted() = runNodeTest {
    var foo: JsFunction? = JsFunction(this, "foo") { _, _ -> null }
    global!!.set("_foo", foo)
    assertTrue(foo!!.hasValue())
    val reference = WeakReference(foo)
    assertNotNull(reference.get())
    global!!.delete("_foo")
    yield()
    assertTrue(!foo.hasValue())
    foo = null
    withTimeout(50_000) {
      while (reference.get() != null) {
        System.gc()
        delay(1000)
      }
    }
  }
}