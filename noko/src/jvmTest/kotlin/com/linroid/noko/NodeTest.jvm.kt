package com.linroid.noko

import org.junit.Test
import kotlin.test.assertFalse
import kotlin.test.assertNotNull
import kotlin.test.assertTrue

class JvmNodeTest : WithNode() {
  @Test
  fun sharedValue() {
    assertNotNull(node.sharedNull)
    assertNotNull(node.sharedUndefined)
    assertNotNull(node.sharedTrue)
    assertTrue(node.sharedTrue.get())
    assertFalse(node.sharedFalse.get())
  }
}
