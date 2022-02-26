package com.linroid.noko.fs

import com.linroid.noko.Node
import okio.Path

abstract class FileSystem {
  /**
   * Get the mapping path in Node.js by real path
   */
  abstract fun mapping(source: Path): Path

  /**
   * Restore the real path by the mapping path in Node.js
   */
  abstract fun restore(destination: Path): Path

  /**
   * Link this filesystem into runtime
   */
  internal open fun link(node: Node) {}

  enum class Mode(internal val flags: Int) {
    None(0),
    ReadOnly(1),
    WriteOnly(2),
    ReadWrite(3),
  }
}
