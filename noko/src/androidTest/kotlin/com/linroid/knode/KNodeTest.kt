package com.linroid.noko

import androidx.test.platform.app.InstrumentationRegistry
import com.linroid.noko.js.JSContext
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Before
import java.io.File
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * @author linroid
 * @since 2019/11/27
 */
abstract class nokoTest : Noko.LifecycleListener, StdOutput {

  private lateinit var file: File
  private val startLatch = CountDownLatch(1)

  private lateinit var node: Noko
  protected lateinit var context: JSContext

  @Before
  fun setupNode() {
    println("setupNode")
    val appContext = InstrumentationRegistry.getInstrumentation().targetContext
    file = File.createTempFile("tests_", "temp.js")
    node = Noko(appContext.cacheDir, this, keepAlive = true)
    node.start(file.absolutePath)
    node.addListener(this)
    startLatch.await(3, TimeUnit.SECONDS)
    println("Node.js is ready")
  }

  override fun onNodeStart(context: JSContext) {
    super.onNodeStart(context)
    println("onNodeStart")
    this.context = context
    startLatch.countDown()
  }

  @After
  fun destroyNode() {
    println("destroyNode")
    file.delete()
    node.exit(0)
  }

  override fun stdout(str: String) {
    println(str)
  }

  override fun stderr(str: String) {
    println(str)
  }

  protected fun runInNode(action: () -> Unit) {
    runBlocking {
      node.await(action)
    }
  }

  companion object {
    init {
      val appContext = InstrumentationRegistry.getInstrumentation().targetContext
      Noko.setup(appContext)
    }
  }
}
