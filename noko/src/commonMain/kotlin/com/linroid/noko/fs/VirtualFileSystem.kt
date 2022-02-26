package com.linroid.noko.fs

import com.linroid.noko.Node
import com.linroid.noko.Platform
import okio.Path
import okio.Path.Companion.toPath
import kotlin.collections.HashMap

/**
 * A virtual filesystem for Node.js runtime
 *
 * @param root Node.js will aim this directory as it's root directory
 * @param mode The file permissions in default
 */
class VirtualFileSystem(
  private val root: Path,
  private val mode: Mode = Mode.ReadWrite,
) : FileSystem() {

  private val destinationToPoint = HashMap<Path, MountPoint>()

  private var sourceToPoint: Map<Path, MountPoint> = emptyMap()
  private var sortedDestinations: List<Path> = emptyList()
  private var sortedSources: List<Path> = emptyList()

  /**
   * Mount a real [source] file into the virtual filesystem as [destination] path
   *
   * @param source The file path in the real filesystem
   * @param destination The destination path in the virtual filesystem
   * @param mode The access permissions, see [FileSystem.Mode]
   */
  fun mount(destination: Path, source: Path, mode: Mode) {
    check(Platform.fileSystem.exists(source)) { "File doesn't exists: $source" }
    check(destination.isAbsolute) { "The dst path must be an absolute path(starts with '/')" }
    destinationToPoint[destination] = MountPoint(destination, source, mode)
    generatePairs()
  }

  /**
   * Unmount the [destination] file from Node.js
   */
  fun unmount(destination: Path) {
    destinationToPoint.remove(destination)
    generatePairs()
    // TODO: notify native
  }

  /**
   * Sort the path to speed up matching
   */
  private fun generatePairs() {
    sourceToPoint = destinationToPoint.entries.associate { it.value.source to it.value }

    sortedDestinations = destinationToPoint.keys.sortedDescending()
    sortedSources = sourceToPoint.keys.sortedDescending()
  }

  override fun mapping(source: Path): Path {
    val baseSource = sortedSources.find { it.isChildOf(source) }
    if (baseSource != null) {
      val baseDestination = sourceToPoint.getValue(baseSource).destination
      return baseDestination / source.relativeTo(baseSource)
    }
    try {
      return "/".toPath() / source.relativeTo(root)
    } catch (error: IllegalArgumentException) {
      throw IllegalArgumentException("Couldn't find mount point for $source, have you mounted it before?")
    }
  }

  override fun restore(destination: Path): Path {
    val baseDestination = sortedDestinations.find { it.isChildOf(destination) }
    if (baseDestination != null) {
      val baseSource = destinationToPoint.getValue(baseDestination).source
      return baseSource / destination.relativeTo(baseDestination)
    }
    return root / destination.relativeTo(destination.root!!)
  }

  override fun link(node: Node) {
    node.chroot(root)
    destinationToPoint.values.forEach {
      node.mountFile(it.destination, it.source, it.mode)
    }
  }

  private fun Path.isChildOf(base: Path): Boolean {
    return this.toString().startsWith(base.toString())
  }

  private data class MountPoint(
    val destination: Path,
    val source: Path,
    val mode: Mode,
  )
}