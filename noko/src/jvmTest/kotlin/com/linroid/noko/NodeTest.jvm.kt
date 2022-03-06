package com.linroid.noko

import org.junit.Test
import kotlin.test.assertFalse
import kotlin.test.assertNotNull
import kotlin.test.assertTrue

class JvmNodeTest : WithNode() {
  @Test
  fun sharedValue() {
    assertNotNull(node.sharedUndefined)
  }
}
