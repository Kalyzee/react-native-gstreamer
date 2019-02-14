import React from 'react'
import {
    requireNativeComponent,
    View,
    UIManager,
    findNodeHandle,
    StyleSheet,
    Animated,
    DeviceEventEmitter,
    Platform
} from 'react-native'

const PropTypes = require('prop-types')

export const GstState = {
    VOID_PENDING: 0,
    NULL: 1,
    READY: 2,
    PAUSED: 3,
    PLAYING: 4
}

export class GstPlayer extends React.Component {

    currentGstState = undefined
    isPlayerReady = false

    listener = {
        onPlayerInit: undefined,
        onPadAdded: undefined,
        onVolumeChanged: undefined,
        onStateChanged: undefined,
        onUriChanged: undefined,
        onPlayingProgress: undefined,
        onBufferingProgress: undefined,
        onEOS: undefined,
        onElementError: undefined,
        onElementLog: undefined
    }

    constructor(props, context) {
        super(props, context)
    }

    componentDidMount() {
        this.playerHandle = findNodeHandle(this.playerViewRef)

        this.listener.onPlayerInit = DeviceEventEmitter.addListener("onPlayerInit", () => {
            this.onPlayerInit()
        });

        this.listener.onPadAdded = DeviceEventEmitter.addListener("onPadAdded", (padName) => {
            this.onPadAdded(padName)
        });

        this.listener.onVolumeChanged = DeviceEventEmitter.addListener("onVolumeChanged", (volume) => {
            this.onVolumeChanged(volume)
        });

        this.listener.onStateChanged = DeviceEventEmitter.addListener("onStateChanged", (states) => {
            this.onStateChanged(states)
        });

        this.listener.onUriChanged = DeviceEventEmitter.addListener("onUriChanged", (uri_data) => {
            this.onUriChanged(uri_data)
        });

        this.listener.onPlayingProgress = DeviceEventEmitter.addListener("onPlayingProgress", (progress_data) => {
            this.onPlayingProgress(progress_data)
        });

        this.listener.onBufferingProgress = DeviceEventEmitter.addListener("onBufferingProgress", (progress_data) => {
            this.onBufferingProgress(progress_data)
        });

        this.listener.onEOS = DeviceEventEmitter.addListener("onEOS", () => {
            this.onEOS()
        });

        this.listener.onElementError = DeviceEventEmitter.addListener("onElementError", (error_data) => {
            this.onElementError(error_data)
        });

        this.listener.onElementLog = DeviceEventEmitter.addListener("onElementLog", (log_data) => {
            console.log(log_data)
            this.onElementLog(log_data)
        });
    }

    componentDidUpdate(prevProps, prevState) {
        if (prevProps.autoPlay !== this.props.autoPlay && this.props.autoPlay)
            this.play()
    }

    // Callbacks
    onPlayerInit() {
        console.log('Player inited')
        this.isPlayerReady = true

        if (this.props.onPlayerInit)
            this.props.onPlayerInit()
    }

    onPadAdded(name) {
        if (Platform.OS === 'android') {
            name = _message.nativeEvent
        }

        if (this.props.onPadAdded)
            this.props.onPadAdded(name)
    }

    onStateChanged(states) {
        let old_state = undefined;
        let new_state = undefined;

        if (Platform.OS === 'android') {
            old_state = states.nativeEvent.old_state
            new_state = states.nativeEvent.new_state
        } else {
            old_state = states.old_state
            new_state = states.new_state
        }

        this.currentGstState = new_state

        if (this.props.onStateChanged)
            this.props.onStateChanged(old_state, new_state)
    }

    onVolumeChanged(volume) {

        if (Platform.OS === 'android') {
            volume = Object.keys(volume.nativeEvent).filter(key => key !== "target").reduce((obj, key) => {
                obj[key] = volume.nativeEvent[key];
                return obj;
            }, {})
        }

        const audioLevelArray = Object.values(volume)

        if (this.props.onVolumeChanged)
            this.props.onVolumeChanged(audioLevelArray)
    }

    onUriChanged(uri_data) {
        let new_uri = undefined;

        if (Platform.OS === 'android') {
            new_uri = uri_data.nativeEvent.new_uri
        } else {
            new_uri = uri_data.new_uri
        }

        if (this.props.autoPlay)
            this.play()

        if (this.props.onUriChanged)
            this.props.onUriChanged(new_uri)
    }

    onBufferingProgress(progress_data) {
        let progress = undefined;

        if (Platform.OS === 'android') {
            progress = progress_data.nativeEvent.progress
        } else {
            new_uri = progress_data.progress
        }

        if (this.props.onBufferingProgress)
            this.props.onBufferingProgress(progress)
    }

    onPlayingProgress(progress_data) {
        let progress = undefined;
        let duration = undefined;

        if (Platform.OS === 'android') {
            progress = progress_data.nativeEvent.progress
            duration = progress_data.nativeEvent.duration
        } else {
            progress = progress_data.progress
            duration = progress_data.duration
        }

        if (this.props.onPlayingProgress)
            this.props.onPlayingProgress(progress, duration)
    }

    onEOS() {
        if (!this.props.loop)
            this.stop()
        else
            this.seek(0)

        if (this.props.onEOS)
            this.props.onEOS()
    }

    onElementError(error_data) {
        let source = undefined;
        let message = undefined;
        let debug_info = undefined;

        if (Platform.OS === 'android') {
            source = error_data.nativeEvent.source
            message = error_data.nativeEvent.message
            debug_info = error_data.nativeEvent.debug_info
        } else {
            source = error_data.source
            message = error_data.message
            debug_info = error_data.debug_info
        }

        if (this.props.onElementError)
            this.props.onElementError(source, message, debug_info)
    }

    onElementLog(log_data) {
        let message = undefined;
        if (Platform.OS === 'android') {
            message = log_data.nativeEvent.message
        } else {
            message = log_data.message
        }

        if (this.props.onElementLog)
            this.props.onElementLog(message)
    }

    // Methods
    setGstState(state) {
        UIManager.dispatchViewManagerCommand(
            this.playerHandle,
            UIManager.getViewManagerConfig("RCTGstPlayer").Commands.setState,
            [state]
        )
    }

    seek(position) {
        UIManager.dispatchViewManagerCommand(
            this.playerHandle,
            UIManager.getViewManagerConfig("RCTGstPlayer").Commands.seek,
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
                pointerEvents='box-none'
                onLayout={this.props.onLayout}
            >
                <RCTGstPlayer
                    uri={this.props.uri !== undefined ? this.props.uri : ""}
                    shareInstance={this.props.shareInstance !== undefined ? this.props.shareInstance : false}
                    uiRefreshRate={this.props.uiRefreshRate !== undefined ? this.props.uiRefreshRate : 100}
                    volume={this.props.volume !== undefined ? this.props.volume : 1.0}

                    onPlayerInit={this.onPlayerInit.bind(this)}
                    onPadAdded={this.onPadAdded.bind(this)}
                    onStateChanged={this.onStateChanged.bind(this)}
                    onVolumeChanged={this.onVolumeChanged.bind(this)}
                    onUriChanged={this.onUriChanged.bind(this)}
                    onBufferingProgress={this.onBufferingProgress.bind(this)}
                    onPlayingProgress={this.onPlayingProgress.bind(this)}
                    onEOS={this.onEOS.bind(this)}
                    onElementError={this.onElementError.bind(this)}
                    onElementLog={this.onElementLog.bind(this)}

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
    overlayOpacity: PropTypes.number,
    overlayFadeInSpeed: PropTypes.number,
    overlayFadeOutSpeed: PropTypes.number,

    // Events callbacks
    onPlayerInit: PropTypes.func,
    onPadAdded: PropTypes.func,
    onStateChanged: PropTypes.func,
    onVolumeChanged: PropTypes.func,
    onUriChanged: PropTypes.func,
    onBufferingProgress: PropTypes.func,
    onPlayingProgress: PropTypes.func,
    onEOS: PropTypes.func,
    onElementError: PropTypes.func,
    onElementLog: PropTypes.func,

    // Methods
    setGstState: PropTypes.func,
    play: PropTypes.func,
    pause: PropTypes.func,
    stop: PropTypes.func,

    // Helper methods
    createDrawableSurface: PropTypes.func,
    destroyDrawableSurface: PropTypes.func,

    ...View.propTypes
}

const styles = StyleSheet.create({
    playerContainer: {
        flex: 1,
        backgroundColor: '#000'
    },
    player: {
        flex: 1
    },
    overlay: {
        backgroundColor: '#000000',
        position: 'absolute',
        top: 0, left: 0, right: 0, bottom: 0
    }
})

const RCTGstPlayer = requireNativeComponent('RCTGstPlayer', GstPlayer, {
    nativeOnly: { onChange: true }
})
