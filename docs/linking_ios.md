---
layout: doc
---

## Linking with iOS

* Run the usual: <b>react-native link react-native-gstreamer</b> from your project folder

* GStreamer Preparation
    * Get the latest GStreamer Library for iOS at https://gstreamer.freedesktop.org/data/pkg/ios/
    * Install the downloaded package in the default path
    * Running your app now would give you many linker errors...

* Linking with GStreamer library
    * Select your project item in the file explorer
    * Select the project (above TARGETS item)
    * Select the "Build Settings" tab and select "All" displaying
    * Filter with name : "Other linker Flags"
    * Double click slowly on the field to edit it textually
    * Add (copy paste) the following flags : <i>
        -lresolv -lstdc++ -framework CoreFoundation -framework Foundation  -framework AVFoundation -framework CoreMedia -framework CoreVideo -framework CoreAudio -framework AudioToolbox -weak_framework VideoToolbox -framework OpenGLES -framework AssetsLibrary -framework QuartzCore -framework AssetsLibrary</i>
    * Now filter with "Framework Search Paths"
    * Add : "~/Library/Developer/GStreamer/iPhone.sdk"
    * Now select your project (under "TARGETS" item)
    * Go in the "Build Phases" tab
    * Expand "Link Binary With Libraries" section and click "+" Button
    * Click "Add Other..." button and find "GStreamer.framework"
    * Click "+" again, filter "libiconv.2" and Add it

* You are good to go !
