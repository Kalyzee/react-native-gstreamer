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
    gdouble volume;                                                 // Volume of the resource
    
    // Callbacks
    void(*onInit)(void);                                            // Called when the player is ready
    void(*onStateChanged)(GstState old_state, GstState new_state);  // Called method when GStreamer state changes
    void(*onVolumeChanged)(RctGstAudioLevel *audioLevel, gint nb_channels); // Called method when current media volume changes
    void(*onUriChanged)(gchar *new_uri);                            // Called when changing uri is over
    void(*onEOS)(void);                                             // Called when EOS occurs
    void(*onElementError)(gchar *source, gchar *message,            // Called when an error occurs
                          gchar *debug_info);
} RctGstConfiguration;

// User data definition
typedef struct {
    
    // Globals items
    RctGstAudioLevel* audio_level;
    RctGstConfiguration* configuration;
    
    GMainContext *context;
    GstPipeline *pipeline;
    GstElement *uri_decode_bin;
    GMainLoop *main_loop;
    guint bus_watch_id;
    GstBus *bus;
    gboolean is_ready;
    gboolean mustApplyUri;
    gboolean mediaHasAudio;
    gboolean mediaHasVideo;
    
    // Video specifics
    GstVideoOverlay *video_overlay;
    GstElement *video_sink;
    GstElement *video_test_src;
    GstElement *video_fake_src;
    
    // Audio specifics
    GstElement *audio_level_analyser;
    GstElement *audio_volume;
    GstElement *audio_test_src;
    GstElement *audio_fake_src;

    // Inputs selectors (Allow switching between debug mode or uri sourced mde
    GstElement *audio_selector;
    GstElement *video_selector;
    
    // Converters
    GstElement *audio_convertor;
    GstElement *video_convertor;
    
    // Inputs selectors pads
    GstPad *audio_selector_sink_pad;
    GstPad *audio_selector_debug_sink_pad;
    GstPad *audio_selector_fake_sink_pad;
    
    GstPad *video_selector_sink_pad;
    GstPad *video_selector_debug_sink_pad;
    GstPad *video_selector_fake_sink_pad;
    
    // Bins
    GstElement *audio_sink_bin;
    GstElement *video_sink_bin;
    
    // Pads
    GstPad *black_screen_source_selector_pad;
    GstPad *default_source_selector_pad;
} RctGstUserData;

// Setters
void rct_gst_set_uri(RctGstUserData *user_data, gchar* _uri);
void rct_gst_set_audio_level_refresh_rate(RctGstUserData *user_data, gint rct_gst_set_audio_level_refresh_rate);
void rct_gst_set_debugging(RctGstUserData *user_data, gboolean is_debugging);
void rct_gst_set_volume(RctGstUserData *user_data, gdouble volume);
void rct_gst_set_drawable_surface(RctGstUserData *user_data, guintptr drawable_surface);

RctGstUserData *rct_gst_init_user_data();
void rct_gst_free_user_data(RctGstUserData* user_data);

// Other
GstStateChangeReturn rct_gst_set_pipeline_state(RctGstUserData *user_data, GstState state);
void rct_gst_init(RctGstUserData *user_data);
void rct_gst_run_loop(RctGstUserData *user_data);
void rct_gst_terminate(RctGstUserData *user_data);

gchar *rct_gst_get_info();
void rct_gst_apply_uri(RctGstUserData *user_data);
void rct_gst_refresh_active_pads(RctGstUserData *user_data);

#endif /* gstreamer_backend_h */
