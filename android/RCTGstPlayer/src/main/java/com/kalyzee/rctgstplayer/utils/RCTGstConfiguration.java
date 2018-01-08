package com.kalyzee.rctgstplayer.utils;

import android.view.Surface;

/**
 * Created by asapone on 02/01/2018.
 */

public class RCTGstConfiguration {

    // Uri of the resource
    private String uri;
    public String getUri() {
        return uri;
    }
    public void setUri(String uri) {
        this.uri = uri;
    }

    // Time in ms between each call of onVolumeChanged
    private int audioLevelRefreshRate;
    public int getAudioLevelRefreshRate() {
        return audioLevelRefreshRate;
    }
    public void setAudioLevelRefreshRate(int audioLevelRefreshRate) {
        this.audioLevelRefreshRate = audioLevelRefreshRate;
    }

    // Pointer to drawable surface
    private Surface initialDrawableSurface;
    public Surface getInitialDrawableSurface() {
        return initialDrawableSurface;
    }
    public void setInitialDrawableSurface(Surface initialDrawableSurface) {
        this.initialDrawableSurface = initialDrawableSurface;
    }

    // Loads debugging pipeline
    private boolean isDebugging;
    public boolean isDebugging() {
        return isDebugging;
    }
    public void setDebugging(boolean debugging) {
        isDebugging = debugging;
    }

    // Callbacks implementations
    private RCTGstConfigurationCallable RCTGstConfigurationCallable;
    public RCTGstConfiguration(RCTGstConfigurationCallable RCTGstConfigurationCallable) {
        this.RCTGstConfigurationCallable = RCTGstConfigurationCallable;
    }
}