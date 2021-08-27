plugins {
  id("com.android.library")
  kotlin("android")
}

android {
  compileSdkVersion(Builds.compileSdkVersion)

  defaultConfig {
    minSdkVersion(Builds.minSdkVersion)
    targetSdkVersion(Builds.compileSdkVersion)

    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    consumerProguardFiles(file("consumer-rules.pro"))
    externalNativeBuild {
      cmake {
        cppFlags += "-std=c++11 -fexceptions"
        arguments += "-DANDROID_STL=c++_shared"

        if (!Builds.isRelease) {
          cppFlags += " -DNODE_DEBUG"
        }
      }
    }

    ndk {
      abiFilters.addAll(Builds.abiFilters)
    }
  }

  buildTypes {
    getByName("release") {
      isMinifyEnabled = false
      proguardFiles(
        getDefaultProguardFile("proguard-android.txt"),
        file("proguard-rules.pro")
      )
    }
  }

  externalNativeBuild {
    cmake {
      path = file("src/main/cpp/CMakeLists.txt")
      version = "3.10.2"
    }
  }

  packagingOptions {
    jniLibs {
      if (!Builds.isRelease) {
        keepDebugSymbols += "**/*.so"
      }
    }
  }
}

dependencies {
  implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
  implementation(Libs.Jetpack.annotation)
  implementation(Libs.gson)
  implementation(Libs.Jetpack.collection)
  groupImplementation(Libs.Kotlin)
  groupImplementation(Libs.Coroutine)

  testImplementation(Libs.Test.junit)
  testImplementation(Libs.Test.mockio)
  testImplementation(Libs.Test.mockioKotliln) {
    exclude(module = "mockito-core")
  }
  androidTestImplementation(Libs.Jetpack.Test.junit)
  androidTestImplementation(Libs.Jetpack.Test.espressoCore)
  androidTestImplementation(Libs.Test.mockio)
  androidTestImplementation(Libs.Test.mockioKotliln) {
    exclude(module = "mockito-core")
  }
}
