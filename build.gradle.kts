plugins {
  id("com.android.library")
  kotlin("android")
}


android {
  compileSdkVersion(30)

  defaultConfig {
    minSdkVersion(23)
    targetSdkVersion(30)
    versionCode = 1
    versionName = "1.0"

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
      isUseProguard = false
      proguardFiles(
        getDefaultProguardFile("proguard-android.txt"),
        file("proguard-rules.pro")
      )
    }
  }

  externalNativeBuild {
    cmake {
      path = file("src/main/cpp/CMakeLists.txt")
    }
  }

  packagingOptions {
    if (!Builds.isRelease) {
      doNotStrip("*/*/*.so")
    }
  }
}

dependencies {
  implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
  implementation(Libs.Jetpack.annotation)
  implementation(Libs.gson)
  implementation(Libs.Jetpack.collection)
  androidTestImplementation(Libs.Kotlin.stdlib)

  testImplementation(Libs.Test.junit)
  androidTestImplementation(Libs.Jetpack.Test.junit)
  androidTestImplementation(Libs.Jetpack.Test.espressoCore)
  androidTestImplementation(Libs.Kotlin.reflect)
}
