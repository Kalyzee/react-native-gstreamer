import React from 'react'
import {
    requireNativeComponent,
    View,
    UIManager,
    findNodeHandle,
    AppState,
    Platform,
    StyleSheet,
    Text
} from 'react-native'

const PropTypes = require('prop-types')

export const GstState = {
    VOID_PENDING: 0,
    NULL: 1,
    READY: 2,
    PAUSED: 3,
    PLAYING: 4
}

export default class GstPlayer extends React.Component {

    currentGstState = undefined
    isPlayerReady = false

    componentDidMount() {
        this.playerHandle = findNodeHandle(this.playerViewRef)
    }

    // From Debugging to video with autoplay
    componentDidUpdate(prevProps, prevStates) {
        if (this.props.autoPlay) {
            this.play()
        }
    }

    // Callbacks
    onPlayerInit() {
        this.isPlayerReady = true

        if (this.props.onPlayerInit)
            this.props.onPlayerInit()
    }

    onStateChanged(_message) {
        const { old_state, new_state } = _message.nativeEvent
        this.currentGstState = new_state

        if (this.props.onStateChanged)
            this.props.onStateChanged(old_state, new_state)
    }

    onVolumeChanged(_message) {
        const audioLevelObject = Object.keys(_message.nativeEvent).filter(key => key !== "target").reduce( (obj, key) => {
            obj[key] = _message.nativeEvent[key];
            return obj;
        }, {})

        let audioLevelArray = Object.values(audioLevelObject)

        if (this.props.onVolumeChanged)
            this.props.onVolumeChanged(audioLevelArray)
    }

    onUriChanged(_message) {
        const { new_uri } = _message.nativeEvent

        if (this.props.autoPlay)
            this.play()

        if (this.props.onUriChanged)
            this.props.onUriChanged(new_uri)
    }

    onBufferingProgress(_message) {
        const { progress } = _message.nativeEvent

        if (this.props.onBufferingProgress)
            this.props.onBufferingProgress(progress)
    }

    onPlayingProgress(_message) {
        const { progress, duration } = _message.nativeEvent

        if (this.props.onPlayingProgress)
            this.props.onPlayingProgress(progress, duration)
    }

    onEOS() {
        if (!this.props.loop)
            this.pause()
        else
            this.seek(0)

        if (this.props.onEOS)
            this.props.onEOS()
    }

    onElementError(_message) {
        const { source, message, debug_info } = _message.nativeEvent

        if (this.props.onElementError)
            this.props.onElementError(source, message, debug_info)
    }

    // Methods
    setGstState(state) {
        UIManager.dispatchViewManagerCommand(
            this.playerHandle,
            UIManager.RCTGstPlayer.Commands.setState,
            [state]
        )
    }

    seek(position) {
        UIManager.dispatchViewManagerCommand(
            this.playerHandle,
            UIManager.RCTGstPlayer.Commands.seek,
            [position]
        )
    }

    // Player state shortcuts
    play() {
        this.setGstState(GstState.PLAYING)
    }

    pause() {
        this.setGstState(GstState.PAUSED)
    }

    stop() {
        this.setGstState(GstState.READY)
    }

    isReady() {
        return this.isPlayerReady
    }

    render() {
        return (
            <View
                style={[styles.playerContainer, this.props.containerStyle]}
            >
                <RCTGstPlayer
                    uri={this.props.uri !== undefined ? this.props.uri : ""}
                    shareInstance={this.props.shareInstance !== undefined ? this.props.shareInstance : false}
                    uiRefreshRate={this.props.uiRefreshRate !== undefined ? this.props.uiRefreshRate : 100}
                    volume={this.props.volume !== undefined ? this.props.volume : 1.0}

                    onPlayerInit={this.onPlayerInit.bind(this)}
                    onStateChanged={this.onStateChanged.bind(this)}
                    onVolumeChanged={this.onVolumeChanged.bind(this)}
                    onUriChanged={this.onUriChanged.bind(this)}
                    onBufferingProgress={this.onBufferingProgress.bind(this)}
                    onPlayingProgress={this.onPlayingProgress.bind(this)}
                    onEOS={this.onEOS.bind(this)}
                    onElementError={this.onElementError.bind(this)}

                    ref={(playerView) => this.playerViewRef = playerView}

                    style={[styles.player, this.props.playerStyle]}
                />
            </View>
        )
    }
}

GstPlayer.propTypes = {

    // Props
    uri: PropTypes.string,
    autoPlay: PropTypes.bool,
    loop: PropTypes.bool,
    uiRefreshRate: PropTypes.number,
    shareInstance: PropTypes.bool,
    volume: PropTypes.number,

    // Events callbacks
    onPlayerInit: PropTypes.func,
    onStateChanged: PropTypes.func,
    onVolumeChanged: PropTypes.func,
    onUriChanged: PropTypes.func,
    onBufferingProgress: PropTypes.func,
    onPlayingProgress: PropTypes.func,
    onEOS: PropTypes.func,
    onElementError: PropTypes.func,

    // Methods
    setGstState: PropTypes.func,
    play: PropTypes.func,
    pause: PropTypes.func,
    stop: PropTypes.func,

    // Helper methods
    createDrawableSurface: PropTypes.func,
    destroyDrawableSurface: PropTypes.func
}

const styles = StyleSheet.create({
    playerContainer: {
        flex: 1
    },
    player: {
        flex: 1
    }
})

const RCTGstPlayer = requireNativeComponent('RCTGstPlayer', GstPlayer, {
    nativeOnly: { onChange: true }
})