//
//  gstreamer_backend.h
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#ifndef gstreamer_backend_h
#define gstreamer_backend_h

#include <stdio.h>
#include <math.h>
#include <gst/gst.h>
#include <gst/video/video.h>

// Audio level definition
typedef struct {
    gdouble rms;
    gdouble peak;
    gdouble decay;
} RctGstAudioLevel;

// Plugin configurator
typedef struct
{
    gchar *uri;                                                     // Uri of the resource
    gint *audioLevelRefreshRate;                                    // Time in ms between each call of onVolumeChanged
    guintptr drawableSurface;                                       // Pointer to drawable surface
    gboolean isDebugging;                                           // Loads debugging pipeline
    
    // Callbacks
    void(*onInit)(void);                                            // Called when the player is ready
    void(*onStateChanged)(GstState old_state, GstState new_state);  // Called method when GStreamer state changes
    void(*onVolumeChanged)(RctGstAudioLevel* audioLevel);           // Called method when current media volume changes
    void(*onUriChanged)(gchar *new_uri);                            // Called when changing uri is over
    void(*onEOS)(void);                                             // Called when EOS occurs
    void(*onElementError)(gchar *source, gchar *message,            // Called when an error occurs
                          gchar *debug_info);
} RctGstConfiguration;

// Getters
RctGstConfiguration *rct_gst_get_configuration();
RctGstAudioLevel *rct_gst_get_audio_level();

// Setters
void rct_gst_set_drawable_surface(guintptr _drawableSurface);
void rct_gst_set_uri(gchar* _uri);
void rct_gst_set_audio_level_refresh_rate(gint rct_gst_set_audio_level_refresh_rate);
void rct_gst_set_debugging(gboolean is_debugging);

// Other
GstStateChangeReturn rct_gst_set_pipeline_state(GstState state);
void rct_gst_init();
void rct_gst_run_loop();
void rct_gst_terminate();

gchar *rct_gst_get_info();
void rct_gst_apply_drawable_surface();
void rct_gst_apply_uri();

#endif /* gstreamer_backend_h */
