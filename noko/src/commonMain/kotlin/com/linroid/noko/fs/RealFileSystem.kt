package com.linroid.noko.fs

import java.io.File

class RealFileSystem : FileSystem() {

  override fun path(file: File): String {
    return file.absolutePath
  }

  override fun file(path: String): File {
    return File(path)
  }
}
