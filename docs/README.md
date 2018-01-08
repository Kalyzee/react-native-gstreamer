# react-native-gstreamer
React native GStreamer is an audio/video player built for react-native using GStreamer framework.
It handles everything GStreamer can natively handle. For more information, you can go here : https://gstreamer.freedesktop.org/

## Features
* Plays anything a GStreamer playbin can play
* Hardware accelerated decoding for better performances and extended battery life
* Great with any kind of media, it was initially though for low latency streaming (RTSP)
* Working for both Android and IOS

## Work left to do
* Add a default graphical control bar (You can create your own one using control methods if needed)
* Allow seeking in media (Nothing is done about this yet)
* Allow multichannel audio level inspection (only mono analysis yet)

## Installation
    npm install --save react-native-gstreamer

## How to link to your project
<span style="color:red"><b>/!\ Be sure to read everything carefully : GStreamer is a  C Library. It will be necessary to finalize the linking manually.</b></span>

Once installed : 
* Be sure you have at least one run your app through <b>react-native run-android</b> cli so your project is initialized correctly for Android platform
* Run the usual: <b>react-native link react-native-gstreamer</b> from your project folder

#### Android

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

##### iOS
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


## Basic usage
```js
import React, { Component } from 'react'
import { StyleSheet, View, Button } from 'react-native'
import GstPlayer, { GstState } from 'react-native-gstreamer'

export default class App extends Component {

    uri1 = "http://mirrors.standaloneinstaller.com/video-sample/jellyfish-25-mbps-hd-hevc.mp4"
    uri2 = "http://mirrors.standaloneinstaller.com/video-sample/Panasonic_HDC_TM_700_P_50i.mp4"

    constructor(props, context) {
        super(props, context)
        this.state = { uri: this.uri1 }
    }

    render() {
        return (
            <View style={styles.container}>
                <GstPlayer
                    style={styles.videoPlayer}
                    uri={this.state.uri}
                    ref="GstPlayer"
                    autoPlay={true}
                />
                <View style={styles.controlBar}>
                    <Button title="uri1" onPress={() => this.setState({ uri: this.uri1 })}></Button>
                    <Button title="uri2" onPress={() => this.setState({ uri: this.uri2 })}></Button>
                    <Button title="Stop" onPress={() => this.refs.GstPlayer.stop()}></Button>
                    <Button title="Pause" onPress={() => this.refs.GstPlayer.pause()}></Button>
                    <Button title="Play" onPress={() => this.refs.GstPlayer.play()}></Button>
                </View>
            </View>
        )
    }
}

const styles = StyleSheet.create({
    container: { flex: 1 },
    controlBar: {
        flexDirection: "row",
        justifyContent: "space-between"
    },
    videoPlayer: { flex: 1 }
})
```

## Available properties

#### Parameters
| Parameter             | Type    | Default   | Description                                                                                                                                                                                                                    |
|-----------------------|---------|-----------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| uri                   | String  | undefined | Path to the desired media to play                                                                                                                                                                                              |
| autoPlay              | Boolean | false     | Will automatically start playing a media when the player is ready, or when you change a media uri                                                                                                                              |
| audioLevelRefreshRate | Integer | 100       | Defines the frequency of audio volume analysis in milliseconds. Helpful to design a vumeter                                                                                                                                    |
| isDebugging           | Boolean | false     | When set to true, it will show a videotestsrc instead of a default playbin. Helpful to check if an issue is coming from playbin or not. Please note that for now, you need to restart the player in order to apply this change |

#### Callbacks
| Method                                                           | Description                                                                                                                            |
|------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------|
| onPlayerInit()                                                   |  Called when the player is ready. Could be useful to display/hide a loading notification                                               |
| onStateChanged(GstState old_state, GstState new_state)           | Called when GStreamer engine is done switching state. You should use GstState enum import to switch on values                          |
| onVolumeChanged(Double rms, Double peak, Double decay)           | Contains audio volume between 0 and 1. It is called as often as defined in audioLevelRefreshRate parameter                             |
| onUriChanged(String new_uri)                                     | Called when GStreamer engine is done changing an uri. Helpful to display a loading notification                                        |
| onEOS()                                                          | Called when a media stream is ended                                                                                                    |
| onElementError(String source, String message, String debug_info) | Called when an internal component of GStreamer playbin's pipeline has met an error. It can be helpful to debug any media playing issue |

#### Methods
| Method                      | Description                                                                                                    |
|-----------------------------|----------------------------------------------------------------------------------------------------------------|
| setGstState(GstState state) | Call this if you want to call GStreamer states yourself. You should use GstState enum import to select a state |
| play()                      | Plays the current media (Alias for setGstState to GstState.PLAYING)                                            |
| pause()                     | Pauses the current media (Alias for setGstState to GstState.PAUSED)                                            |
| stop()                      | Stops the current media (Alias for setGstState to GstState.READY)                                              |