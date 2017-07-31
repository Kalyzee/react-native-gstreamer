# react-native-gstreamer

React native GStreamer is an audio/video player built for react-native using GStreamer framework.
It handles everything GStreamer can natively handle. For more informations, you can go here : https://gstreamer.freedesktop.org/

## IOS

### Dependencies

You need <b>xcode</b> to build anything.
You also need the GStreamer framework. To do so, please download and install the package : <b>GStreamer 1.12.2</b> you can get from https://gstreamer.freedesktop.org/data/pkg/ios/

### How to build

You can open the project with <b>xcode</b> :
 - Open the file : <b>ios/GStreamerIOS.xcodeproj</b>

### Resources

Gstreamer tutorial : https://gstreamer.freedesktop.org/documentation/tutorials/ios/index.html

## Android

### Dependencies

 - You need Android NDK, Revision 10e (May 2015) available here : Resources we used for making this react native component : https://developer.android.com/ndk/downloads/older_releases.html. Because there is an issue in NDK r14 and r15 which prevent us to build gstreamer.

### Resources

 - Gstreamer tutorial : https://gstreamer.freedesktop.org/documentation/tutorials/android/
 - Gstreamer graddle : https://github.com/jaroslavas/Gstreamer-Android-example

## Documentation

###  Installation

 > npm install --save

###  Usage

```
import { GSTPlayer } from 'react-native-gstreamer'
...
render: function() {
    let uri = "your-uri"
    return (
        <GSTPlayer
            uri={uri}
        />
    )
}
```

###  Options

| Option | Effect                          |
|:------:|---------------------------------|
|   uri  | Defines the source of the media |