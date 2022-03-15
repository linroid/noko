// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
  // id("com.android.application") version "7.3.0-alpha06" apply false
  // id("org.jetbrains.kotlin.android") version "1.6.10" apply false
  // id("com.github.ben-manes.versions").version("0.36.0")
}

buildscript {
  repositories {
    mavenCentral()
    google()
    gradlePluginPortal()
    maven("https://maven.pkg.jetbrains.space/public/p/compose/dev")
  }

  dependencies {
    classpath("com.android.tools.build:gradle:7.3.0-alpha05")
    classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.6.10")
    classpath("de.undercouch:gradle-download-task:5.0.2")
    classpath("org.jetbrains.compose:compose-gradle-plugin:1.2.0-alpha01-dev620")
  }
}

allprojects {
  repositories {
    mavenCentral()
    google()
  }
}
