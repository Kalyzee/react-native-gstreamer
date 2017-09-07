package com.kalyzee.gstreamer;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.freedesktop.gstreamer.GStreamer;

public class GstPlayer extends SurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "ReactNative";
    static private final int PICK_FILE_CODE = 1;

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("gstreamer");
        nativeClassInit();
    }

    private final String defaultMediaUri = "http://ftp.nluug.nl/pub/graphics/blender/demo/movies/Sintel.2010.1080p.mkv";
    private boolean isReady;
    private int position;                 // Current position, reported by native code
    private int duration;                 // Current clip duration, reported by native code
    private boolean is_local_media;       // Whether this clip is stored locally or is being streamed
    private int desired_position;         // Position where the users wants to seek to
    private String mediaUri;              // URI of the clip being played
    private String last_folder;
    private long native_custom_data;      // Native code will use this to keep private data
    private boolean is_playing_desired = true;   // Whether the user asked to go to PLAYING
    public GstPlayer(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        try {
            GStreamer.init(getContext());
        } catch (Exception e) {
        }
        mediaUri = defaultMediaUri;
        SurfaceHolder sh = this.getHolder();
        sh.addCallback(this);
        nativeInit();
    }
    public GstPlayer(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }
    public GstPlayer(Context context) {
        this(context, null);
    }

    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks

    private native void nativeInit();     // Initialize native code, build pipeline, etc

    private native void nativeFinalize(); // Destroy pipeline and shutdown native code

    //private PowerManager.WakeLock wake_lock;

    private native void nativePlay();     // Set pipeline to PLAYING

    private native void nativePause();    // Set pipeline to PAUSED

    private native void nativeSetUri(String uri); // Set the URI of the media to play

    private native void nativeSetPosition(int milliseconds); // Seek to the indicated position, in milliseconds

    private native void nativeSurfaceInit(Object surface);

    private native void nativeSurfaceFinalize();

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "Surface changed to format " + format + " width " + width + " height " + height);
    }

    // Set the URI to play, and record whether it is a local or remote file
    public void setMediaUri(String uri) {
        this.mediaUri = uri;
        nativeSetUri(mediaUri);
    }

    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "Surface created: " + holder.getSurface());
        nativeSurfaceInit(holder.getSurface());
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "Surface destroyed");
        nativeSurfaceFinalize();
    }

    private void setCurrentPosition(final int position, final int duration) {

    }

    private void onMediaSizeChanged(int width, int height) {
        Log.d(TAG, "onMediaSizeChanged");
    }

    // Called from native code. This sets the content of the TextView from the UI thread.
    private void setMessage(final String message) {

        if (message.equals("State changed to READY")) {
            Log.d(TAG, "READY " + this.mediaUri);
            nativePlay();
        }
    }

    protected void onDestroy() {
        nativeFinalize();
    }

    // Called from native code. Native code calls this once it has created its pipeline and
    // the main loop is running, so it is ready to accept commands.
    private void onGStreamerInitialized() {
        Log.d(TAG, "Gst initialized. Restoring state, playing:" + is_playing_desired);
        nativeSetUri(mediaUri);
        nativeSetPosition(position);
        if (is_playing_desired) {
            nativePlay();
        }
    }

    public void setPlay(boolean play) {
        Log.d(TAG, "setPlay: " + play);
        is_playing_desired = play;

        if (play && isReady) {
            nativePlay();
        } else {
            if (!play && isReady) {
                nativePause();
            }
        }
    }
}
