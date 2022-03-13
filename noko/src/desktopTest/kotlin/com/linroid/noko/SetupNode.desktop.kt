package com.linroid.noko

actual abstract class SetupNode {
  actual companion object {
    init {
      Node.setup(4)
    }
  }
}