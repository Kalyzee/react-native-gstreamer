# react-native-gstreamer
React native GStreamer is an audio/video player built for react-native using GStreamer framework.
It handles everything GStreamer can natively handle. For more information, you can go here : https://gstreamer.freedesktop.org/

## Features
* Plays anything a GStreamer playbin can play
* Hardware accelerated decoding for better performances and extended battery life
* Great with any kind of media, it was initially though for low latency streaming (RTSP)
* Working for both Android and IOS

## Work left to do
* Volume control
* Add a default graphical control bar (You can create your own one using control methods if needed)
* Allow seeking in media (Nothing is done about this yet)
* Allow multichannel audio level inspection (only mono analysis yet)

## Installation
    npm install --save react-native-gstreamer

## How to link to your project
<span style="color:red"><b>/!\ Be sure to read everything carefully : GStreamer is a  C Library. It will be necessary to finalize the linking manually.</b></span>
    
* [Linking for Android](./docs/linking_android.md)
* [Linking for iOS](./docs/linking_ios.md)

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

