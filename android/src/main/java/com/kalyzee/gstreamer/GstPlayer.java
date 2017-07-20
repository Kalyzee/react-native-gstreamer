package com.kalyzee.gstreamer;

import android.content.Context;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.Log;

import android.view.View;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

import org.freedesktop.gstreamer.GStreamer;

public class GstPlayer extends SurfaceView implements SurfaceHolder.Callback {

  public int media_width = 320;
  public int media_height = 240;

  private native void nativeInit();     // Initialize native code, build pipeline, etc
  private native void nativeFinalize(); // Destroy pipeline and shutdown native code
  private native void nativePlay();     // Set pipeline to PLAYING
  private native void nativePause();    // Set pipeline to PAUSED
  private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
  private native void nativeSurfaceInit(Object surface);
  private native void nativeSurfaceFinalize();
  private long native_custom_data;      // Native code will use this to keep private data
  private boolean is_playing_desired= true;   // Whether the user asked to go to PLAYING

  public GstPlayer(Context context, AttributeSet attrs,
          int defStyle) {
      super(context, attrs, defStyle);
      try {
          GStreamer.init(getContext());
      } catch (Exception e) {
      }

      SurfaceHolder sh = this.getHolder();
      sh.addCallback(this);
      nativeInit();
  }

  public GstPlayer(Context context, AttributeSet attrs) {
      this(context, attrs, 0);
  }

  public GstPlayer (Context context) {
      this(context, null);
  }


  public void surfaceChanged(SurfaceHolder holder, int format, int width,
          int height) {
      Log.i("GStreamer", "Surface changed to format " + format + " width "
              + width + " height " + height);
      nativeSurfaceInit (holder.getSurface());
  }

  public void surfaceCreated(SurfaceHolder holder) {
      Log.i("GStreamer", "Surface created: " + holder.getSurface());
  }

  public void surfaceDestroyed(SurfaceHolder holder) {
      Log.i("GStreamer", "Surface destroyed");
      nativeSurfaceFinalize ();
  }

  // Called from native code. This sets the content of the TextView from the UI thread.
  private void setMessage(final String message) {
    if (message.equals("State changed to READY")){
      nativePlay();
    }
  }

  // Called from native code. Native code calls this once it has created its pipeline and
  // the main loop is running, so it is ready to accept commands.
  private void onGStreamerInitialized () {
      Log.i ("GStreamer", "Gst initialized. Restoring state, playing:" + is_playing_desired);
      if (is_playing_desired) {
          nativePlay();
      } else {
          nativePause();
      }

  }


  static {
      System.loadLibrary("gstreamer_android");
      System.loadLibrary("gstreamer");
      nativeClassInit();

  }

}
