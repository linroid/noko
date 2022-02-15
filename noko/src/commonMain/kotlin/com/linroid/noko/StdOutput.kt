package com.linroid.noko

interface StdOutput {
  val supportsColor: Boolean
    get() = false

  fun stdout(str: String)

  fun stderr(str: String)
}
