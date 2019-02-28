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
    void *owner;                                                    // Reference to instance
    
    // Callbacks
    void(*onPlayerInit)(void *owner);                                                       // Called when the player is ready
    void(*onPadAdded)(void *owner, gchar *name);                                            // Called when pad is created
    void(*onStateChanged)(void *owner, GstState old_state, GstState new_state);             // Called method when GStreamer state changes
    void(*onVolumeChanged)(void *owner, RctGstAudioLevel *audioLevel, gint nb_channels);    // Called method when current media volume changes
    void(*onUriChanged)(void *owner, gchar *new_uri);                                       // Called when changing uri is over
    void(*onPlayingProgress)(void *owner, gint64 progress, gint64 duration);                // Called when playing progression changed / duration is defined
    void(*onBufferingProgress)(void *owner, gint progress);                                 // Called when buffering progression changed
    void(*onEOS)(void *owner);                                                              // Called when EOS occurs
    void(*onElementError)(void *owner, gchar *source, gchar *message,                       // Called when an error occurs
                          gchar *debug_info);
    void(*onElementLog)(void *owner, gchar *message);                                       // Called when an log occurs
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
    GstElement *video_queue;
    GstElement *video_sink;
    GstElement *video_depay;
    GstElement *h264parse;
    GstElement *rtpjitterbuffer;
    GstElement *h264dec;

    // Audio
    GstBin *audio_sink_bin;
    GstBin *video_sink_bin;
    GstElement *audio_level_analyser;
    GstElement *audio_queue;
    GstElement *audio_sink;
    GstElement *volume_controller;
    GstElement *audio_depay;
    
    // Misc
    GstElement *source;
    gboolean must_apply_uri;
    gboolean is_ready;
    gboolean must_clear_screen;
    gint reconnect_retry_count;
    
    // Media informations and seeking
    gint64 duration;
    gint64 position;
    gint64 desired_position;     // Position to seek to, once the pipeline is running
    GstClockTime last_seek_time; // For seeking overflow prevention (throttling)
    
} RctGstUserData;

RctGstUserData *rct_gst_init_user_data();
void rct_gst_free_user_data(RctGstUserData* user_data);

// Callbacks
static gboolean cb_delayed_seek(RctGstUserData* user_data);
static void cb_eos(GstBus *bus, GstMessage *msg, RctGstUserData* user_data);

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

// Utils
static gboolean rct_gst_element_has_attribute(GstElement *element, const gchar *attribute);
void rct_gst_log(RctGstUserData *user_data, gchar* message);

#endif /* gstreamer_backend_h */
