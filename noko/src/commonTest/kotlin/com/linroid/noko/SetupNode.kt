package com.linroid.noko

abstract class SetupNode {
  companion object {
    init {
      Node.setup(4)
    }
  }
}
