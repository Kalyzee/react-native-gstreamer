package com.kalyzee.rctgstplayer;

import android.support.annotation.Nullable;
import android.util.Log;
import android.view.View;

import com.facebook.react.bridge.ReadableArray;
import com.facebook.react.common.MapBuilder;
import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.annotations.ReactProp;
import com.kalyzee.rctgstplayer.utils.manager.Command;

import java.util.Map;

/**
 * Created by asapone on 02/01/2018.
 */

public class RCTGstPlayer extends SimpleViewManager<View> {

    private static RCTGstPlayerController playerController;

    @Override
    public String getName() {
        return "RCTGstPlayer";
    }

    @Override
    protected View createViewInstance(ThemedReactContext reactContext) {
        if (playerController == null)
        {
            playerController = new RCTGstPlayerController(reactContext);
            reactContext.addLifecycleEventListener(playerController);
        }


        return playerController.getView();
    }

    // Shared properties
    @ReactProp(name = "uri")
    public void setUri(View controllerView, String uri) {
        Log.d(RCTGstPlayerController.LOG_TAG, "ReactProp uri : " + uri);
        playerController.setRctGstUri(uri);
    }

    @ReactProp(name = "volume")
    public void setVolume(View controllerView, double volume) {
        Log.d(RCTGstPlayerController.LOG_TAG, "ReactProp volume : " + volume);
        playerController.setRctGstVolume(volume);
    }

    @ReactProp(name = "uiRefreshRate")
    public void setUiRefreshRate(View controllerView, int uiRefreshRate) {
        Log.d(RCTGstPlayerController.LOG_TAG, "ReactProp refreshRate : " + uiRefreshRate);
        playerController.setRctGstUiRefreshRate(uiRefreshRate);
    }

    // Methods
    @Override
    public void receiveCommand(View view, int commandType, @Nullable ReadableArray args) {

        // setState
        if (Command.is(commandType, Command.setState)) {
            playerController.setRctGstState(args.getInt(0));
        }

        // seek
        if (Command.is(commandType, Command.seek)) {
            playerController.seek(args.getInt(0));
        }

        // destroy
        if (Command.is(commandType, Command.destroy)) {
            playerController.destroy();
        }
    }

    // Commands (JS callable methods listing map)
    @Override
    public Map<String, Integer> getCommandsMap() {
        return Command.getCommandsMap();
    }

    /**
     * This method maps the sending of the "onAudioLevelChange" event to the JS "onAudioLevelChange" function.
     */

    // Events callbacks
    @Nullable @Override
    public Map<String, Object> getExportedCustomDirectEventTypeConstants() {
        return MapBuilder.<String, Object>builder()
                .put(
                        "onPlayerInit", MapBuilder.of("registrationName", "onPlayerInit")
                ).put(
                        "onStateChanged", MapBuilder.of("registrationName", "onStateChanged")
                ).put(
                        "onPlayingProgress", MapBuilder.of("registrationName", "onPlayingProgress")
                ).put(
                        "onVolumeChanged", MapBuilder.of("registrationName", "onVolumeChanged")
                ).put(
                        "onUriChanged", MapBuilder.of("registrationName", "onUriChanged")
                ).put(
                        "onEOS", MapBuilder.of("registrationName", "onEOS")
                ).put(
                        "onElementError", MapBuilder.of("registrationName", "onElementError")
                ).build();
    }
}
