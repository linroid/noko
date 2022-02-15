package com.linroid.noko

/**
 * @author linroid
 * @since 2019-10-14
 */
interface StdOutput {
  val supportsColor: Boolean
    get() = false

  fun stdout(str: String)

  fun stderr(str: String)
}
