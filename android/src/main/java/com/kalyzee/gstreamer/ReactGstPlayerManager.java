package com.kalyzee.gstreamer;

import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.annotations.ReactProp;
import com.kalyzee.gstreamer.GstPlayer;


public class ReactGstPlayerManager extends SimpleViewManager<GstPlayer> {

    public static final String REACT_CLASS = "GstPlayer";

    @Override
    public String getName() {
        return REACT_CLASS;
    }

    @Override
    protected GstPlayer createViewInstance(
            ThemedReactContext reactContext) {
        GstPlayer gst = new GstPlayer(reactContext);
        return gst;
    }

    @ReactProp(name = "uri")
    public void setUri(GstPlayer view,
                       String uri) {
        view.setMediaUri(uri);
    }

    @ReactProp(name = "play")
    public void setPlayState(GstPlayer view,
                             boolean play) {
        view.setPlay(play);
    }


}
