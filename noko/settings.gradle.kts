pluginManagement {
  repositories {
    google()
    gradlePluginPortal()
    mavenCentral()
  }
  resolutionStrategy {
    eachPlugin {
      if (requested.id.namespace == "com.android") {
        useModule("com.android.tools.build:gradle:7.1.1")
      } else if (requested.id.id == "kotlinx-atomicfu") {
        useModule("org.jetbrains.kotlinx:atomicfu-gradle-plugin:0.17.1")
      }
    }
  }
}

rootProject.name = "noko"
