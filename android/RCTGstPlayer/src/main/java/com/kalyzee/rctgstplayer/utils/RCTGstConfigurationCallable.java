package com.kalyzee.rctgstplayer.utils;

import com.kalyzee.rctgstplayer.utils.RCTGstAudioLevel;

/**
 * Created by asapone on 04/01/2018.
 */

public interface RCTGstConfigurationCallable {
    // Called when the player is ready
    void onPlayerInit();

    // Called when pad is created
    void onPadAdded(String name);

    // Called method when GStreamer state changes
    void onStateChanged(int old_state, int new_state);

    // Called method when current media volume changes
    void onVolumeChanged(RCTGstAudioLevel[] audioLevel, int nbChannels);

    // Called when changing uri is over
    void onUriChanged(String new_uri);

    // Called when playing progression changed / duration is defined
    void onPlayingProgress(long progress, long duration);

    // Called when buffering progression changed
    void onBufferingProgress(double progress);

    // Called when EOS occurs
    void onEOS();

    // Called when an error occurs
    void onElementError(String source, String message, String debug_info);

    // Called when an log occurs
    void onElementLog(String message);
}
