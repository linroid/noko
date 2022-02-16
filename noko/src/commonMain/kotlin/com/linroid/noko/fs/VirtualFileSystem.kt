package com.linroid.noko.fs

import com.linroid.noko.Noko
import java.io.File
import java.util.*
import kotlin.collections.HashMap

/**
 * A virtual filesystem for Node.js runtime
 *
 * @param root Node.js will aim this directory as it's root directory
 * @param mode The file permissions in default
 */
class VirtualFileSystem(
  private val root: String,
  private val mode: FileMode = FileMode.ReadWrite,
) : FileSystem() {
  private val points = HashMap<String, MountPoint>()

  /**
   * Longer path should be matched firstly
   */
  private val dst2src = TreeMap<String, String> { a, b -> b.compareTo(a) }
  private val src2dst = TreeMap<String, String> { a, b -> b.compareTo(a) }

  /**
   * Mount a real [src] file/directory into the virtual filesystem as [dst] path
   *
   * @param src The [File] in the real filesystem
   * @param dst The destination path in the virtual filesystem
   * @param mode The permissions for accessing this path, see [FileMode]
   * @param mapping If true, a placeholder file will be created if the [dst] path related to [root]
   * doesn't exists in real filesystem, this can make it visible when listing files
   */
  fun mount(dst: String, src: String, mode: FileMode, mapping: Boolean = true) {
    val srcFile = File(src)
    check(srcFile.exists()) { "File doesn't exists: $src" }
    check(dst.startsWith("/")) { "The dst path must be an absolute path(starts with '/')" }
    points[dst] = MountPoint(dst, src, mode)
    generatePairs()
    if (mapping) {
      if (srcFile.isDirectory) {
        val dir = File(root, dst.substring(0))
        if (!dir.exists()) {
          dir.mkdirs()
        }
      } else {
        srcFile.parentFile?.mkdirs()
        val file = File(root, dst.substring(0))
        if (!file.exists()) {
          file.createNewFile()
          when (mode) {
            FileMode.ReadOnly -> {
              file.setReadOnly()
            }
            FileMode.WriteOnly -> {
              file.setReadable(false)
              file.setWritable(true)
            }
            FileMode.ReadWrite -> {
              file.setReadable(true)
              file.setWritable(true)
            }
            else -> {}
          }
        }
      }
    }
  }

  private fun generatePairs() {
    src2dst.clear()
    dst2src.clear()
    points.values.forEach {
      src2dst[it.src] = it.dst
      src2dst[it.dst] = it.src
    }
  }

  override fun path(file: String): String {
    val baseFile = src2dst.keys.find { file.startsWith(it) }
    if (baseFile != null) {
      return "/" + file.relativeTo(baseFile)
    }
    try {
      return "/" + file.relativeTo(root)
    } catch (error: IllegalArgumentException) {
      throw IllegalArgumentException("$file doesn't belong to any mount points")
    }
  }

  override fun file(path: String): String {
    val basePath = dst2src.keys.find { path.startsWith(it) }
    if (basePath != null) {
      return dst2src.getValue(basePath) + path.relativeTo(basePath)
    }
    return root + path.relativeTo("/")
  }

  private fun String.relativeTo(base: String): File {
    return File(this).relativeTo(File(base))
  }

  override fun link(noko: Noko) {
    noko.chroot(root)
    points.values.forEach {
      noko.mountFile(it.dst, it.src, it.mode)
    }
  }

  private data class MountPoint(
    val dst: String,
    val src: String,
    val mode: FileMode,
  )
}