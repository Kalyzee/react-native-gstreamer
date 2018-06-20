---
layout: doc
---

## Linking with Android

* Run the usual: <b>react-native link react-native-gstreamer</b> from your project folder

* Be sure you have at least one run your app through <b>react-native run-android</b> cli so your project is initialized correctly

* GStreamer Preparation
    * Get the latest GStreamer Library for Android at https://gstreamer.freedesktop.org/data/pkg/android/
    * Extract it wherever you like
    
* Android Studio Preparation
    * Open Android Studio
    * Go to "Tools -> Android -> SDK Manager"
    * Go to the SDK Tools tab
    * Be sure to have CMake, LLDB, NDK installed and up to date

* Project update
    * Open your project in Android Studio
    * Upgrade gradle (4.1 ATM) when prompted if you wish to be synchronized with this tutorial
    * Fix Gradle wrapper and re-import project if requested
    * Edit settings.gradle (change the projectDir) :
    ```gradle
    include ':react-native-gstreamer'
    project(':react-native-gstreamer').projectDir = new File(rootProject.projectDir, '../node_modules/react-native-gstreamer/android/RCTGstPlayer')
    ```
    * Re-sync Gradle. It might prompt you to update your maven repo. Let it do it the dirty job for you.

* Replace GStreamer library path in your project
    * Open the build.gradle from your project module
    * Add the following to the end of your file :
    ```gradle
    project.ext.set("gstAndroidRoot", "/absolute/path/to/gstreamer/android/library")
    ```
    * Note that the required path is the one containing all the platforms

* You are good to go !
