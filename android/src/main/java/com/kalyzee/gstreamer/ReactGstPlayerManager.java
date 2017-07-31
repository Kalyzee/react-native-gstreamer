package com.kalyzee.gstreamer;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.widget.ProgressBar;
import com.kalyzee.gstreamer.GstPlayer;
import android.widget.Toast;

import com.facebook.drawee.backends.pipeline.Fresco;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.ViewManager;
import com.facebook.react.uimanager.ViewProps;
import com.facebook.react.uimanager.annotations.ReactProp;
import com.facebook.react.views.image.ImageResizeMode;
import com.facebook.react.views.image.ReactImageManager;
import com.facebook.react.views.image.ReactImageView;

import java.util.Arrays;
import java.util.List;


public class ReactGstPlayerManager extends SimpleViewManager<GstPlayer> {

  public static final String REACT_CLASS = "GstPlayer";

  @Override
  public String getName() {
    return REACT_CLASS;
  }

  public ReactGstPlayerManager(){
    super();
    Log.i("KAST", "KASTOK");
  }

  @Override
  protected GstPlayer createViewInstance(
          ThemedReactContext reactContext) {
    Log.i("KAST", "CREATE VIEW createViewInstance");
    GstPlayer gst = new GstPlayer(reactContext);
    Log.i("KAST", "CREATE VIEW createViewInstance 2");

    return gst;
  }

  @ReactProp(name = "uri")
  public void setUri(GstPlayer view,
                               String uri) {
    view.setMediaUri(uri);
  }


}
