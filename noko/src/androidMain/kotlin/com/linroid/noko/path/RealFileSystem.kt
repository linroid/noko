package com.linroid.noko.path

import com.linroid.noko.noko
import java.io.File

/**
 * @author linroid
 * @since 3/21/21
 */
class RealFileSystem : FileSystem {

  override fun path(file: File): String {
    return file.absolutePath
  }

  override fun file(path: String): File {
    return File(path)
  }

  override fun mount(node: noko) {
    // do nothing
  }
}
