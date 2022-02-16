package com.linroid.noko.fs

import com.linroid.noko.Noko

abstract class FileSystem {
  /**
   * Get the virtual path for [file]
   */
  abstract fun path(file: String): String

  /**
   * Get the real file from [path]
   */
  abstract fun file(path: String): String

  /**
   * Link this filesystem into runtime
   */
  internal open fun link(noko: Noko) {}
}
