package com.linroid.noko.fs

import com.linroid.noko.Noko
import java.io.File

abstract class FileSystem {
  /**
   * Get the virtual path for [file]
   */
  abstract fun path(file: File): String

  /**
   * Get the real file from [path]
   */
  abstract fun file(path: String): File

  /**
   * Link this filesystem into runtime
   */
  internal open fun link(noko: Noko) {}
}
