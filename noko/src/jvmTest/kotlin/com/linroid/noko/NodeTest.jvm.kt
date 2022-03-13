package com.linroid.noko

import org.junit.Test
import kotlin.test.assertNotNull

class JvmNodeTest : SetupNode() {
  @Test
  fun sharedValue() = runNodeTest {
    assertNotNull(sharedUndefined)
  }
}
