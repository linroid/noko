import com.android.build.gradle.LibraryExtension
import com.android.build.gradle.tasks.ExternalNativeBuildTask
import de.undercouch.gradle.tasks.download.Download
import org.jetbrains.kotlin.gradle.dsl.KotlinMultiplatformExtension

plugins {
  kotlin("multiplatform") version "1.6.10"
  kotlin("plugin.serialization") version "1.6.10"
  id("com.android.library")
  id("de.undercouch.download") version "5.0.1"
  // id("kotlinx-atomicfu")
}

group = "com.linroid.noko"
version = "1.0-SNAPSHOT"

val coroutinesVersion = "1.6.0"

repositories {
  google()
  mavenCentral()
}

// Remove useless source sets
// See https://discuss.kotlinlang.org/t/disabling-androidandroidtestrelease-source-set-in-gradle-kotlin-dsl-script/21448
afterEvaluate {
  extensions.getByType<KotlinMultiplatformExtension>().sourceSets.removeAll { sourceSet ->
    setOf(
      "androidAndroidTestRelease",
      "androidTestFixtures",
      "androidTestFixturesDebug",
      "androidTestFixturesRelease",
    ).contains(sourceSet.name)
  }
}

android {
  compileSdk = 31
  sourceSets["main"].manifest.srcFile("src/androidMain/AndroidManifest.xml")

  defaultConfig {
    minSdk = 24
    targetSdk = 31
    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    consumerProguardFiles(file("consumer-rules.pro"))
    externalNativeBuild {
      cmake {
        cppFlags += "-std=c++11 -fexceptions"
        arguments += setOf("-DANDROID_ARM_MODE=arm", "-DANDROID_STL=c++_shared")
      }
    }
  }

  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
  }

  externalNativeBuild {
    cmake {
      path = file("src/jvmMain/cpp/CMakeLists.txt")
      version = "3.10.2"
    }
  }

  packagingOptions {
    jniLibs {
      keepDebugSymbols += "**/*.so"
    }
  }
}

java {
  sourceCompatibility = JavaVersion.VERSION_1_8
  targetCompatibility = JavaVersion.VERSION_1_8
}

val nodeVersion = project.property("libnode.version")
val downloadsDir = File(buildDir, "downloads")
val osName = System.getProperty("os.name")!!
val targetOs = when {
  osName == "Mac OS X" -> "macos"
  osName.startsWith("Win") -> "windows"
  osName.startsWith("Linux") -> "linux"
  else -> error("Unsupported OS: $osName")
}

val targetArch = when (val osArch = System.getProperty("os.arch")) {
  "x86_64", "amd64" -> "x86_64"
  "aarch64" -> "arm64"
  else -> error("Unsupported arch: $osArch")
}

val prebuiltDir = file("src/jvmMain/cpp/prebuilt")
val hostPrebuiltDir = File(prebuiltDir, "$targetOs/$targetArch")
val hostPrebuiltLibDir = File(hostPrebuiltDir, "lib")
val cmakeDir = File(buildDir, "$targetOs/cmake")
cmakeDir.mkdirs()

tasks {
  val downloadAndroidPrebuilt by registering(Download::class) {
    src("https://github.com/linroid/libnode/releases/download/$nodeVersion/libnode-$nodeVersion-android.zip")
    dest(File(downloadsDir, "libnode.android.zip"))
  }

  val targetAndroidPrebuiltDir = file("src/jvmMain/cpp/prebuilt/android")
  val prepareAndroidPrebuilt by registering(Copy::class) {
    dependsOn(downloadAndroidPrebuilt)
    from(zipTree(File(downloadsDir, "libnode.android.zip")))
    into(targetAndroidPrebuiltDir)
  }

  val downloadHostPrebuilt by registering(Download::class) {
    src("https://github.com/linroid/libnode/releases/download/$nodeVersion/libnode-$nodeVersion-$targetOs-$targetArch.zip")
    dest(File(downloadsDir, "libnode.$targetOs.zip"))
  }

  val prepareHostPrebuilt by registering(Copy::class) {
    dependsOn(downloadHostPrebuilt)
    from(zipTree(File(downloadsDir, "libnode.$targetOs.zip")))
    into(hostPrebuiltDir)
  }

  val hostCmakeConfigure by registering(Exec::class) {
    workingDir(cmakeDir)
    commandLine("cmake", file("src/jvmMain/cpp/"))
  }

  val hostCmakeBuild by registering(Exec::class) {
    if (!hostPrebuiltDir.exists()) {
      dependsOn(prepareHostPrebuilt)
    }
    dependsOn(hostCmakeConfigure)
    workingDir(cmakeDir)
    commandLine("make")
    doLast {
      val libNoko = cmakeDir.listFiles()!!.find { it.name.startsWith("libnoko") }
      checkNotNull(libNoko) { "Couldn't find file: libnoko" }
      val libNode = hostPrebuiltLibDir.listFiles()!!.find {
        it.name.startsWith("libnode")
      }
      checkNotNull(libNode) { "Couldn't find file: libnode" }
      project.tasks.withType(Jar::class).forEach {
        it.from(libNoko)
        it.from(libNode)
      }
    }
  }

  named("clean").configure {
    doLast {
      delete(prebuiltDir)
    }
  }

  afterEvaluate {
    getByName("compileKotlinDesktop").dependsOn(hostCmakeBuild)
    if (!targetAndroidPrebuiltDir.exists()) {
      tasks.withType(ExternalNativeBuildTask::class).forEach {
        it.dependsOn(prepareAndroidPrebuilt)
      }
    }
  }
  withType<Test> {
    testLogging {
      outputs.upToDateWhen { false }
      showStandardStreams = true
    }
  }
}

kotlin {
  android {}

  jvm("desktop") {
    compilations.all {
      kotlinOptions.jvmTarget = "11"
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
      jvmArgs("-Djava.library.path=$cmakeDir:$hostPrebuiltLibDir")
    }
  }

  sourceSets {
    val mockkVersion = "1.12.3"
    val commonMain by getting {
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("com.squareup.okio:okio:3.0.0")
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.3.2")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:$coroutinesVersion")
        implementation("org.jetbrains.kotlinx:atomicfu:0.17.1")
      }
    }
    val commonTest by getting {
      dependencies {
        implementation("io.mockk:mockk-common:${mockkVersion}")
        implementation(kotlin("test"))
      }
    }

    val jvmMain by creating {
      dependsOn(commonMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-core-jvm:1.3.2")
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json-jvm:1.3.2")
        implementation("org.jetbrains.kotlinx:atomicfu-jvm:0.17.1")
        implementation("com.squareup.okio:okio-jvm:3.0.0")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:$coroutinesVersion")
      }
    }

    val jvmTest by creating {
      dependsOn(commonTest)
      dependencies {
        implementation(kotlin("test"))
        implementation("io.mockk:mockk:${mockkVersion}")
        implementation("io.mockk:mockk-agent-jvm:${mockkVersion}")
      }
    }

    val desktopMain by getting {
      dependsOn(jvmMain)
    }

    val desktopTest by getting {
      dependsOn(jvmTest)
      dependencies {
        implementation("io.mockk:mockk:${mockkVersion}")
      }
    }

    val androidMain by getting {
      dependsOn(jvmMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:$coroutinesVersion")
      }
    }
    val androidTest by getting {
      dependsOn(jvmTest)
      dependencies {
        implementation("junit:junit:4.13.2")
      }
    }
  }
}

fun configureSourceSets() {
  val libraryExtension = project.extensions.findByType<LibraryExtension>()!!
  // TODO: b/148416113: AGP doesn't know about Kotlin-MPP's sourceSets yet, so add
  // them to its source directories (this fixes lint, and code completion in
  // Android Studio on versions >= 4.0canary8)
  libraryExtension.apply {
    sourceSets.findByName("main")?.apply {
      kotlin.srcDirs("src/androidMain/kotlin")
      res.srcDirs("src/androidMain/res")
      assets.srcDirs("src/androidMain/assets")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
    sourceSets.findByName("test")?.apply {
      kotlin.srcDirs("src/androidTest/kotlin")
      res.srcDirs("src/commonTest/res", "src/jvmTest/res")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
    sourceSets.findByName("androidTest")?.apply {
      java.srcDirs("src/androidAndroidTest/kotlin")
      res.srcDirs("src/androidAndroidTest/res")
      assets.srcDirs("src/androidAndroidTest/assets")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
  }
}

afterEvaluate {
  configureSourceSets()
}
