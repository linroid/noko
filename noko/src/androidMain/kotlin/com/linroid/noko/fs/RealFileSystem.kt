package com.linroid.noko.fs

import com.linroid.noko.Noko
import java.io.File

class RealFileSystem : FileSystem {

  override fun path(file: File): String {
    return file.absolutePath
  }

  override fun file(path: String): File {
    return File(path)
  }

  override fun mount(node: Noko) {
    // do nothing
  }
}
