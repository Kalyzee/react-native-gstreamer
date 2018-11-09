package com.kalyzee.rctgstplayer.utils;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by asapone on 02/01/2018.
 */

public class EaglUIView extends SurfaceView {

    public EaglUIView(Context context, SurfaceHolder.Callback surfaceHolderManager) {
        super(context);
        this.getHolder().addCallback(surfaceHolderManager);
    }
}
