import React from 'react'
import {
    requireNativeComponent,
    View,
    UIManager,
    findNodeHandle,
    AppState,
    Platform,
    StyleSheet
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
    appState = "active"
    isInitialized = false

    componentDidMount() {
        this.playerHandle = findNodeHandle(this.playerViewRef)
        AppState.addEventListener('change', this.appStateChanged)
    }

    componentWillUnmount() {
        AppState.removeEventListener('change', this.appStateChanged)
    }

    /*
    shouldComponentUpdate(nextProps, nextState) {
        const { width, height } = this.props
        return width !== nextProps.width || height !== nextProps.height;
    }
    */

    appStateChanged = (nextAppState) => {
        if (this.appState.match(/inactive|background/) && nextAppState === 'active') {
            this.play()
        } else {
            this.stop()
        }
        this.appState = nextAppState
    }

    // Callbacks
    onPlayerInit() {
        this.isInitialized = true

        if (this.props.onPlayerInit)
            this.props.onPlayerInit()

        if (this.props.autoPlay)
            this.play();
    }

    onStateChanged(_message) {
        const { old_state, new_state } = _message.nativeEvent
        this.currentGstState = new_state

        if (this.props.onStateChanged)
            this.props.onStateChanged(old_state, new_state)
    }

    onVolumeChanged(_message) {
        const { rms, peak, decay } = _message.nativeEvent

        if (this.props.onVolumeChanged)
            this.props.onVolumeChanged(rms, peak, decay)
    }

    onUriChanged(_message) {
        const { new_uri } = _message.nativeEvent

        if (this.props.onUriChanged) {
            this.props.onUriChanged(new_uri)
        }

        if (this.props.autoPlay) {
            this.play()
        }
    }

    onEOS() {
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

    render() {
        return (
            <View style={
                [styles.playerContainer, this.props.style]
            }>
                <RCTGstPlayer
                    onLayout={this.onLayout}
                    autoPlay={this.props.autoPlay}
                    uri={this.props.uri || undefined}
                    shareInstance={this.props.shareInstance !== undefined ? this.props.shareInstance : false}
                    audioLevelRefreshRate={this.props.audioLevelRefreshRate !== undefined ? this.props.audioLevelRefreshRate : 100}
                    isDebugging={this.props.isDebugging !== undefined ? this.props.isDebugging : false}
                    volume={this.props.volume !== undefined ? this.props.volume : 1.0}

                    onPlayerInit={this.onPlayerInit.bind(this)}
                    onStateChanged={this.onStateChanged.bind(this)}
                    onVolumeChanged={this.onVolumeChanged.bind(this)}
                    onUriChanged={this.onUriChanged.bind(this)}
                    onEOS={this.onEOS.bind(this)}
                    onElementError={this.onElementError.bind(this)}

                    ref={(playerView) => this.playerViewRef = playerView}

                    style={{ flex: 1 }}
                />
            </View>
        )
    }
}

GstPlayer.propTypes = {

    // Props
    uri: PropTypes.string,
    autoPlay: PropTypes.bool,
    audioLevelRefreshRate: PropTypes.number,
    isDebugging: PropTypes.bool,
    shareInstance: PropTypes.bool,
    volume: PropTypes.number,

    // Events callbacks
    onPlayerInit: PropTypes.func,
    onStateChanged: PropTypes.func,
    onVolumeChanged: PropTypes.func,
    onUriChanged: PropTypes.func,
    onEOS: PropTypes.func,
    onElementError: PropTypes.func,

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
        backgroundColor: 'rgba(0, 0, 0, 0)'
    }
})

const RCTGstPlayer = requireNativeComponent('RCTGstPlayer', GstPlayer, {
    nativeOnly: { onChange: true }
})