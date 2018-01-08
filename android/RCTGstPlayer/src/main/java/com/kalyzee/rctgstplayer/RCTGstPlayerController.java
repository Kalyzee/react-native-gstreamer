package com.kalyzee.rctgstplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.Toast;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.uimanager.events.RCTEventEmitter;
import com.kalyzee.rctgstplayer.utils.EaglUIView;
import com.kalyzee.rctgstplayer.utils.RCTGstConfigurationCallable;
import com.kalyzee.rctgstplayer.utils.RCTGstConfiguration;

import org.freedesktop.gstreamer.GStreamer;

/**
 * Created by asapone on 02/01/2018.
 */

public class RCTGstPlayerController implements RCTGstConfigurationCallable, SurfaceHolder.Callback {

    private static final String LOG_TAG = "RCTGstPlayer";

    private boolean isInited = false;

    private RCTGstConfiguration configuration;
    private EaglUIView view;
    private ReactContext context;

    // Native methods
    private native String nativeRCTGstGetGStreamerInfo();
    private native void nativeRCTGstSetDrawableSurface(Surface drawableSurface);
    private native void nativeRCTGstSetUri(String uri);
    private native void nativeRCTGstSetAudioLevelRefreshRate(int audioLevelRefreshRate);
    private native void nativeRCTGstSetDebugging(boolean isDebugging);

    private native void nativeRCTGstSetPipelineState(int state);

    private native void nativeRCTGstInitAndRun(RCTGstConfiguration configuration);

    // Configuration callbacks
    @Override
    public void onInit() {
        context.getJSModule(RCTEventEmitter.class).receiveEvent(
                view.getId(), "onPlayerInit", null
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
    public void onVolumeChanged(double rms, double peak, double decay) {
        WritableMap event = Arguments.createMap();

        event.putDouble("rms", rms);
        event.putDouble("peak", peak);
        event.putDouble("decay", decay);

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

    // Surface callbacks
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (!isInited) {

            // Preparing configuration
            this.configuration.setInitialDrawableSurface(holder.getSurface());

            // Init and run our pipeline
            nativeRCTGstInitAndRun(this.configuration);

            // Init done
            this.isInited = true;
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nativeRCTGstSetDrawableSurface(holder.getSurface());
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
            Toast.makeText(context, e.getMessage(), Toast.LENGTH_LONG).show();
        }

        // Display version - For simple debugging purpose
        String version = nativeRCTGstGetGStreamerInfo();
        Log.d(LOG_TAG, version);

        // Create view - surface manager interface is this class
        this.view = new EaglUIView(this.context, this);

        // Create configuration - Callbacks manager interface is this class
        this.configuration = new RCTGstConfiguration(this);
    }

    View getView() {
        return this.view;
    }

    // Manager Shared properties
    void setRctGstUri(String uri) {
        nativeRCTGstSetUri(uri);
    }

    void setRctGstAudioLevelRefreshRate(int audioLevelRefreshRate) {
        nativeRCTGstSetAudioLevelRefreshRate(audioLevelRefreshRate);
    }

    void setRctGstDebugging(boolean isDebugging) {
        nativeRCTGstSetDebugging(isDebugging);
    }

    // Manager methods
    void setRctGstState(int state) {
        nativeRCTGstSetPipelineState(state);
    }

    // External C Libraries
    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("rctgstplayer");
    }
}
