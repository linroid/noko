package com.linroid.noko.fs

import okio.Path

class RealFileSystem : FileSystem() {
  override fun mapping(source: Path): Path = source

  override fun restore(path: Path): Path = path
}
