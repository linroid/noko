// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
  // id("com.github.ben-manes.versions").version("0.36.0")
}

buildscript {
  repositories {
    mavenCentral()
    google()
    // maven("https://kotlin.bintray.com/kotlinx")
    gradlePluginPortal()
  }

  dependencies {
    classpath("com.android.tools.build:gradle:7.3.0-alpha05")
    classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.6.10")
    classpath("de.undercouch:gradle-download-task:5.0.2")
  }
}

allprojects {
  repositories {
    mavenCentral()
    google()
  }
}
