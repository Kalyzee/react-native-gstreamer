package com.kalyzee.rctgstplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.WritableArray;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.uimanager.events.RCTEventEmitter;
import com.kalyzee.rctgstplayer.utils.EaglUIView;
import com.kalyzee.rctgstplayer.utils.RCTGstConfigurationCallable;
import com.kalyzee.rctgstplayer.utils.RCTGstAudioLevel;
import org.freedesktop.gstreamer.GStreamer;

/**
 * Created by asapone on 02/01/2018.
 */

public class RCTGstPlayerController implements RCTGstConfigurationCallable, SurfaceHolder.Callback {

    public static final String LOG_TAG = "RCTGstPlayer";

    private boolean isReady = false;

    private EaglUIView view;
    private ReactContext context;

    // Native methods
    private native String nativeRCTGstGetGStreamerInfo();
    private native void nativeRCTGstInit();
    private native void nativeRCTGstSetDrawableSurface(Surface drawableSurface);
    private native void nativeRCTGstSetUri(String uri);
    private native void nativeRCTGstSetUiRefreshRate(int uiRefreshRate);
    private native void nativeRCTGstSetPipelineState(int state);
    private native void nativeRCTGstSeek(long progress);
    private native void nativeRCTOnPlayerInit();

    // Configuration callbacks
    @Override
    public void onPlayerInit() {
        if (!this.isReady) {
            nativeRCTOnPlayerInit();

            Log.d(LOG_TAG, Integer.toString(view.getId()));
            context.getJSModule(RCTEventEmitter.class).receiveEvent(
                    view.getId(), "onPlayerInit", null
            );
            
            nativeRCTGstSetDrawableSurface(this.view.getHolder().getSurface());

            this.isReady = true;
        }
    }

    @Override
    public void onPadAdded(String name){
        WritableMap event = Arguments.createMap();

        event.putString("name", name);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onPadAdded", event
        );
    }

    @Override
    public void onStateChanged(int old_state, int new_state) {
        WritableMap event = Arguments.createMap();

        event.putInt("old_state", old_state);
        event.putInt("new_state", new_state);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onStateChanged", event
        );
    }

    @Override
    public void onVolumeChanged(RCTGstAudioLevel[] audioLevels, int nbChannels) {
        WritableMap event = Arguments.createMap();
        WritableArray audioLevelArray = Arguments.createArray();

        for (RCTGstAudioLevel audioLevel : audioLevels) {
            WritableMap audioLevelMap = Arguments.createMap();

            audioLevelMap.putDouble("rms", audioLevel.getRms());
            audioLevelMap.putDouble("peak", audioLevel.getPeak());
            audioLevelMap.putDouble("decay", audioLevel.getDecay());

            audioLevelArray.pushMap(audioLevelMap);
        }

        event.putInt("nbChannels", nbChannels);
        event.putArray("volume", audioLevelArray);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onVolumeChanged", event
        );
    }

    @Override
    public void onUriChanged(String new_uri) {
        WritableMap event = Arguments.createMap();

        event.putString("new_uri", new_uri);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onUriChanged", event
        );
    }

    @Override
    public void onPlayingProgress(long progress, long duration) {
        WritableMap event = Arguments.createMap();

        event.putString("progress", Long.toString(progress));
        event.putString("duration", Long.toString(duration));

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
            view.getId(), "onPlayingProgress", event
        );
    }

    @Override
    public void onBufferingProgress(double progress) {
        WritableMap event = Arguments.createMap();

        event.putDouble("progress", progress);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onBufferingProgress", event
        );
    }

    @Override
    public void onEOS() {
        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onEOS", null
        );
    }

    @Override
    public void onElementError(String source, String message, String debug_info) {
        WritableMap event = Arguments.createMap();

        event.putString("source", source);
        event.putString("message", message);
        event.putString("debug_info", debug_info);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onElementError", event
        );
    }

    @Override
    public void onElementLog(String message) {
        WritableMap event = Arguments.createMap();

        event.putString("message", message);

        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onElementLog", event
        );
    }

    // Surface callbacks
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(LOG_TAG, "Surface Created");
        if (!this.isReady) {
            nativeRCTGstInit();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(LOG_TAG, "Surface Changed");
        if (this.isReady) {
            nativeRCTGstSetDrawableSurface(holder.getSurface());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {}

    // Constructor
    public RCTGstPlayerController(ReactContext context) {

        this.context = context;

        // Init GStreamer
        try {
            GStreamer.init(context);
        } catch (Exception e) {
            // Toast.makeText(context, e.getMessage(), Toast.LENGTH_LONG).show();
            Log.e(LOG_TAG, e.toString());
        }

        // Display version - For simple debugging purpose
        String version = nativeRCTGstGetGStreamerInfo();
        Log.d(LOG_TAG, version);
    }

    View getView() {
        if (!this.isReady) {
            // Create view - surface manager interface is this class
            this.view = new EaglUIView(this.context, this);
        }
        return this.view;
    }

    // Manager Shared properties
    void setRctGstUri(String uri) {
        Log.d(LOG_TAG, "setRctGstUri" + uri);
        if (uri.length() > 0) {
            nativeRCTGstSetUri(uri);
        } else {
            Log.e(LOG_TAG, "Uri undefined");
        }
    }

    void setRctGstUiRefreshRate(int uiRefreshRate) {
        nativeRCTGstSetUiRefreshRate(uiRefreshRate);
    }

    // Manager methods
    void setRctGstState(int state) {
        nativeRCTGstSetPipelineState(state);
    }

    void seek(long progress) {
        nativeRCTGstSeek(progress);
    }

    // External C Libraries
    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("kalyzee-rctgstplayer");
    }
}
