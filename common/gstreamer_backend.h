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


// Do not allow seeks to be performed closer than this distance. It is visually useless, and will probably confuse some demuxers.
#define SEEK_MIN_DELAY (500 * GST_MSECOND)

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
    guint64 uiRefreshRate;                                          // Time in ms between each call of onVolumeChanged
    guintptr drawableSurface;                                       // Pointer to drawable surface
    gdouble volume;                                                 // Volume of the resource
    
    // Callbacks
    void(*onPlayerInit)(void);                                      // Called when the player is ready
    void(*onStateChanged)(GstState old_state, GstState new_state);  // Called method when GStreamer state changes
    void(*onVolumeChanged)(RctGstAudioLevel *audioLevel, gint nb_channels); // Called method when current media volume changes
    void(*onUriChanged)(gchar *new_uri);                            // Called when changing uri is over
    void(*onPlayingProgress)(gint64 progress, gint64 duration);     // Called when playing progression changed / duration is defined
    void(*onBufferingProgress)(gint progress);                      // Called when buffering progression changed
    void(*onEOS)(void);                                             // Called when EOS occurs
    void(*onElementError)(gchar *source, gchar *message,            // Called when an error occurs
                          gchar *debug_info);
} RctGstConfiguration;

// User data definition
typedef struct {
    
    // Globals items
    RctGstConfiguration *configuration;
    
    GstPipeline *playbin;
    GMainLoop *main_loop;
    guint bus_watch_id;
    GSource *timeout_source;
    GstBus *bus;
    GstState current_state;
    
    // Video
    guintptr video_overlay;
    
    // Audio
    GstBin *audio_sink_bin;
    GstElement *audio_level_analyser;
    GstElement *audio_sink;
    
    // Misc
    gboolean must_apply_uri;
    gboolean is_ready;
    
    // Media informations and seeking
    gint64 duration;
    gint64 position;
    gint64 desired_position;     // Position to seek to, once the pipeline is running
    GstClockTime last_seek_time; // For seeking overflow prevention (throttling)
    
} RctGstUserData;

RctGstUserData *rct_gst_init_user_data();
void rct_gst_free_user_data(RctGstUserData* user_data);

// Cb
static gboolean cb_delayed_seek(RctGstUserData* user_data);

// Other
GstStateChangeReturn rct_gst_set_playbin_state(RctGstUserData* user_data, GstState state);
void rct_gst_init(RctGstUserData *user_data);
void rct_gst_run_loop(RctGstUserData *user_data);
void rct_gst_terminate(RctGstUserData *user_data);

gchar *rct_gst_get_info();
static void rct_gst_apply_uri(RctGstUserData *user_data);
static void execute_seek(RctGstUserData* user_data, gint64 position);
void rct_gst_seek(RctGstUserData *user_data, gint64 position);

// Setters
void rct_gst_set_uri(RctGstUserData *user_data, gchar* _uri);
void rct_gst_set_ui_refresh_rate(RctGstUserData *user_data, guint64 audio_level_refresh_rate);
void rct_gst_set_volume(RctGstUserData *user_data, gdouble volume);
void rct_gst_set_drawable_surface(RctGstUserData *user_data, guintptr drawable_surface);

#endif /* gstreamer_backend_h */
