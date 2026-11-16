// Top-level build file where you can add configuration options common to all sub-projects/modules.

plugins {
    id("com.android.application") version "8.2.2" apply false
    id("org.jetbrains.kotlin.android") version "1.9.20" apply false
}

// Define versions in one place
ext {
    androidGradlePluginVersion = "8.2.2"
    kotlinVersion = "1.9.20"
    compileSdk = 34
    minSdk = 24
    targetSdk = 34
    versionCode = 2
    versionName = "2.0.0"
}