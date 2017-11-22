import React from 'react'
import { requireNativeComponent, View } from 'react-native'

const PropTypes = require('prop-types')

export default class GstPlayer extends React.Component {

    onAudioLevelChange(message) {
        const audio_level = message.nativeEvent.level
        if (!this.props.onAudioLevelChange)
            return

        this.props.onAudioLevelChange(audio_level)
    }

    render() {
        return (
            <RCTGstPlayer
                {...this.props}
                onAudioLevelChange={this.onAudioLevelChange.bind(this)}
            />
        )
    }
}
GstPlayer.propTypes = {
    uri: PropTypes.string,
    play: PropTypes.bool,
    onAudioLevelChange: PropTypes.func,
    ...View.propTypes
}

var RCTGstPlayer = requireNativeComponent('GstPlayer', GstPlayer, {
    nativeOnly: { onChange: true }
})