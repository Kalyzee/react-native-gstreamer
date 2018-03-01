//
//  gstreamer_backend.c
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#include "gstreamer_backend.h"
#include <stdlib.h>

// User data
RctGstUserData *rct_gst_init_user_data()
{
    // Create user data structure
    RctGstUserData *user_data = calloc(1, sizeof(RctGstUserData));
    
    // Create configuration structure
    user_data->configuration = calloc(1, sizeof(RctGstConfiguration));
    user_data->configuration->uri = "";
    user_data->configuration->uiRefreshRate = -1;
    
    // Other init flags
    user_data->must_apply_uri = FALSE;
    user_data->is_ready = FALSE;
    user_data->current_state = GST_STATE_VOID_PENDING;
    
    user_data->duration = GST_CLOCK_TIME_NONE;
    user_data->position = GST_CLOCK_TIME_NONE;
    user_data->desired_position = GST_CLOCK_TIME_NONE;
    user_data->last_seek_time = gst_util_get_timestamp();
    
    return user_data;
}

void rct_gst_free_user_data(RctGstUserData *user_data)
{
    free(user_data->configuration->uri);
    
    free(user_data->configuration);
    free(user_data);
}

/**********************
 AUDIO HANDLING METHODS
 *********************/
void create_audio_sink_bin(RctGstUserData *user_data)
{
    // Create elements
    user_data->audio_sink_bin = gst_bin_new("audio-sink-bin");
    user_data->audio_level_analyser = gst_element_factory_make("level", "audio-level-analyser");
    user_data->audio_sink = gst_element_factory_make("autoaudiosink", "audio-sink");
    
    // Add them
    gst_bin_add_many(GST_BIN(user_data->audio_sink_bin),
                     user_data->audio_level_analyser,
                     user_data->audio_sink,
                     NULL);
    
    // Link them
    if(!gst_element_link(user_data->audio_level_analyser, user_data->audio_sink))
        g_printerr("RCTGstPlayer : Failed to link audio-level-analyser and audio-sink\n");
    
    // Creating ghostpad for uri-decode-bin
    gst_element_add_pad(GST_BIN(user_data->audio_sink_bin), gst_ghost_pad_new("sink", gst_element_get_static_pad(user_data->audio_level_analyser, "sink")));
}

/*********************
 APPLICATION CALLBACKS
 ********************/

static GstBusSyncReply cb_create_window(GstBus *bus, GstMessage *message, RctGstUserData* user_data)
{
    if(!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;
    
    if (user_data->video_overlay) {
        gst_video_overlay_set_window_handle(user_data->video_overlay, user_data->configuration->drawableSurface);
    }
    
    gst_message_unref(message);
    return GST_BUS_DROP;
}

static void cb_error(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    GError *err;
    gchar *debug_info;
    
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("RCTGstPlayer : Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    
    if (user_data->configuration->onElementError) {
        user_data->configuration->onElementError(GST_OBJECT_NAME(msg->src), err->message, debug_info);
    }
    
    g_clear_error(&err);
    g_free(debug_info);
}

static void cb_eos(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    if (user_data->configuration->onEOS) {
        user_data->configuration->onEOS();
    }
}

static gboolean cb_message_element(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    if(msg->type == GST_MESSAGE_ELEMENT)
    {
        const GstStructure *s = gst_message_get_structure(msg);
        const gchar *name = gst_structure_get_name(s);
        
        RctGstAudioLevel *audio_channels_level;
        
        GValueArray *rms_arr, *peak_arr, *decay_arr;
        gdouble rms_dB, peak_dB, decay_dB;
        const GValue *value;
        
        if(g_strcmp0(name, "level") == 0)
        {
            /* the values are packed into GValueArrays with the value per channel */
            const GValue *array_val = gst_structure_get_value(s, "peak");
            
            array_val = gst_structure_get_value(s, "rms");
            rms_arr = (GValueArray *)g_value_get_boxed(array_val);
            
            array_val = gst_structure_get_value(s, "peak");
            peak_arr = (GValueArray *)g_value_get_boxed(array_val);
            
            array_val = gst_structure_get_value(s, "decay");
            decay_arr = (GValueArray *)g_value_get_boxed(array_val);
            
            // No multichannel needs to be handled - Otherwise : gint channels = rms_arr->n_values;
            gint nb_channels = rms_arr->n_values;
            
            if (nb_channels > 0) {
                audio_channels_level = calloc(nb_channels, sizeof(RctGstAudioLevel));
                
                for (int i = 0; i < nb_channels; i++) {
                    // Create audio level structure
                    RctGstAudioLevel *audio_level_structure = &audio_channels_level[i];
                    
                    // RMS
                    value = g_value_array_get_nth(rms_arr, i);
                    rms_dB = g_value_get_double(value);
                    audio_level_structure->rms = rms_dB; //pow(10, rms_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
                    
                    // PEAK
                    value = g_value_array_get_nth(peak_arr, i);
                    peak_dB = g_value_get_double(value);
                    audio_level_structure->peak = peak_dB; //pow(10, peak_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
                    
                    // DECAY
                    value = g_value_array_get_nth(decay_arr, i);
                    decay_dB = g_value_get_double(value);
                    audio_level_structure->decay = decay_dB; // pow(10, decay_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
                }
                
                if (user_data->configuration->onVolumeChanged) {
                    user_data->configuration->onVolumeChanged(audio_channels_level, nb_channels);
                }
            }
            
            free(audio_channels_level);
        }
    }
    return TRUE;
}

static gboolean cb_message_buffering(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    gint percent;
    gst_message_parse_buffering (msg, &percent);
    
    if (user_data->configuration->onBufferingProgress)
        user_data->configuration->onBufferingProgress(percent);
    
    return TRUE;
}

// Remove latency as much as possible
static void cb_setup_source(GstElement *pipeline, GstElement *source, RctGstUserData* user_data) {
    user_data->duration = GST_CLOCK_TIME_NONE;
    g_object_set(source, "latency", 0, NULL);
}

static gboolean cb_duration_and_progress(RctGstUserData* user_data) {
    
    if (user_data->current_state == GST_STATE_PLAYING) {
        // If we didn't know it yet, query the stream duration
        if (!GST_CLOCK_TIME_IS_VALID (user_data->duration))
            gst_element_query_duration(user_data->playbin, GST_FORMAT_TIME, &user_data->duration);
        
        if (gst_element_query_position(user_data->playbin, GST_FORMAT_TIME, &user_data->position)) {
            if (user_data->configuration->onPlayingProgress)
                user_data->configuration->onPlayingProgress(user_data->position / GST_MSECOND, user_data->duration / GST_MSECOND);
        }
    }
    
    return TRUE;
}

static void cb_state_changed(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    
    
    // Message coming from the playbin
    if(GST_MESSAGE_SRC(msg) == GST_OBJECT(user_data->playbin) && new_state != old_state) {
        
        user_data->current_state = new_state;
        
        g_print("RCTGstPlayer : New playbin state %s\n", gst_element_state_get_name(new_state));
        
        if (new_state == GST_STATE_READY) {
            
            if (!user_data->is_ready) {
                user_data->is_ready = TRUE;
                
                // Get video overlay
                GstElement *video_sink = gst_bin_get_by_interface(GST_BIN(user_data->playbin), GST_TYPE_VIDEO_OVERLAY);
                user_data->video_overlay = GST_VIDEO_OVERLAY(video_sink);
                g_object_unref(video_sink);
                
                if (user_data->configuration->onPlayerInit) {
                    user_data->configuration->onPlayerInit();
                }
            }
            
            // We apply the uri when the uri-decode-bin is in ready state and an uri is set if needed (async step to ready)
            if (g_strcmp0("", user_data->configuration->uri) != 0) {
                if (user_data->must_apply_uri) {
                    rct_gst_apply_uri(user_data);
                    user_data->must_apply_uri = FALSE;
                }
            }
        }
        
        if (new_state >= GST_STATE_READY) {
            rct_gst_set_volume(user_data, user_data->configuration->volume);
        }
        
        // Get element duration if not known yet
        if (new_state >= GST_STATE_PAUSED) {
            if (!GST_CLOCK_TIME_IS_VALID (user_data->duration)) {
                gst_element_query_duration (user_data->playbin, GST_FORMAT_TIME, &user_data->duration);
                
                if (user_data->configuration->onPlayingProgress) {
                    user_data->configuration->onPlayingProgress(0, user_data->duration / GST_MSECOND);
                }
            }
        }
        
        // Seek if requested
        if (new_state == GST_STATE_PLAYING && user_data->desired_position != GST_CLOCK_TIME_NONE) {
            if (GST_CLOCK_TIME_IS_VALID(user_data->desired_position)) {
                
                // Restore good base time as we never stored the right one
                GstClockTime base_time = gst_element_get_base_time(user_data->playbin);
                gst_element_set_base_time(user_data->playbin, base_time - user_data->desired_position);
                
                execute_seek(user_data, user_data->desired_position);
            }
        }
        
        if (user_data->configuration->onStateChanged) {
            user_data->configuration->onStateChanged(old_state, new_state);
        }
    }
}

static gboolean cb_delayed_seek(RctGstUserData* user_data) {
    execute_seek(user_data, user_data->desired_position);
    return FALSE;
}

static gboolean cb_bus_watch(GstBus *bus, GstMessage *message, RctGstUserData* user_data)
{
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
            cb_error(bus, message, user_data);
            break;
            
        case GST_MESSAGE_EOS:
            cb_eos(bus, message, user_data);
            break;
            
        case GST_MESSAGE_STATE_CHANGED:
            cb_state_changed(bus, message, user_data);
            break;
            
        case GST_MESSAGE_ELEMENT:
            cb_message_element(bus, message, user_data);
            break;
            
        case GST_MESSAGE_BUFFERING:
            cb_message_buffering(bus, message, user_data);
            break;
            
        case GST_MESSAGE_DURATION_CHANGED:
            user_data->duration = GST_CLOCK_TIME_NONE;
            cb_duration_and_progress(user_data);
            break;
            
        default:
            break;
    }
    
    return TRUE;
}

/*************
 OTHER METHODS
 ************/
GstStateChangeReturn rct_gst_set_playbin_state(RctGstUserData* user_data, GstState state)
{
    GstStateChangeReturn validity = GST_STATE_CHANGE_FAILURE;
    GstState current_state;
    GstState pending_state;
    
    gst_element_get_state(user_data->playbin, &current_state, &pending_state, 0);
    
    g_print(
            "Actual state : %s - Pipeline state requested : %s - Pending state : %s\n",
            gst_element_state_get_name(current_state),
            gst_element_state_get_name(state),
            gst_element_state_get_name(pending_state)
            );
    
    if (user_data->playbin != NULL && current_state != state) {
        if (pending_state != state) {
            validity = gst_element_set_state(user_data->playbin, state);
        } else {
            g_print("RCTGstPlayer : Ignoring state change (Already changing to requested state)\n");
        }
    }
    
    return validity;
}

void rct_gst_init(RctGstUserData* user_data)
{
    // Create a playbin pipeline
    user_data->playbin = gst_element_factory_make("playbin", "playbin");
    
    // Create audio with level analyser
    create_audio_sink_bin(user_data);
    g_object_set(user_data->playbin, "audio-sink", user_data->audio_sink_bin, NULL);
    
    // Preparing bus
    user_data->bus = gst_element_get_bus(user_data->playbin);
    user_data->bus_watch_id = gst_bus_add_watch(user_data->bus, cb_bus_watch, user_data);
    gst_bus_set_sync_handler(user_data->bus,(GstBusSyncHandler)cb_create_window, user_data, NULL);
    gst_object_unref(user_data->bus);
    
    g_signal_connect(G_OBJECT(user_data->playbin), "source-setup", (GCallback) cb_setup_source, user_data);
    
    // Create loop
    user_data->main_loop = g_main_loop_new(NULL, FALSE);
    
    rct_gst_set_playbin_state(user_data, GST_STATE_READY);
}

void rct_gst_run_loop(RctGstUserData* user_data)
{
    g_main_loop_run(user_data->main_loop);
    g_main_loop_unref(user_data->main_loop);
    
    gst_element_set_state (user_data->playbin, GST_STATE_NULL);
    
    gst_object_unref(user_data->playbin);
    gst_object_unref(user_data->video_overlay);
    
    g_source_remove(user_data->bus_watch_id);
    
    rct_gst_free_user_data(user_data);
}

void rct_gst_terminate(RctGstUserData* user_data)
{
    g_main_loop_quit(user_data->main_loop);
}

gchar *rct_gst_get_info()
{
    return gst_version_string();
}

static void rct_gst_apply_uri(RctGstUserData* user_data)
{
    g_print("RCTGstPlayer : rct_gst_apply_uri\n");
    g_object_set(user_data->playbin, "uri", user_data->configuration->uri, NULL);
    
    if (user_data->configuration->onUriChanged) {
        user_data->configuration->onUriChanged(user_data->configuration->uri);
    }
}

static void execute_seek(RctGstUserData* user_data, gint64 position) {
    gint64 diff;
    
    if (position == GST_CLOCK_TIME_NONE)
        return;
    
    diff = gst_util_get_timestamp() - user_data->last_seek_time;
    
    if (GST_CLOCK_TIME_IS_VALID (user_data->last_seek_time) && diff < SEEK_MIN_DELAY) {
        /* The previous seek was too close, delay this one */
        GSource *timeout_source;
        
        if (user_data->desired_position == GST_CLOCK_TIME_NONE) {
            /* There was no previous seek scheduled. Setup a timer for some time in the future */
            timeout_source = g_timeout_source_new((SEEK_MIN_DELAY - diff) / GST_MSECOND);
            g_source_set_callback(timeout_source, (GSourceFunc)cb_delayed_seek, user_data, NULL);
            g_source_attach(timeout_source, NULL);
            g_source_unref(timeout_source);
        }
        /* Update the desired seek position. If multiple requests are received before it is time
         * to perform a seek, only the last one is remembered. */
        user_data->desired_position = position;
    } else {
        
        /* Perform the seek now */
        user_data->last_seek_time = gst_util_get_timestamp();
        gst_element_seek_simple(
                                user_data->playbin,
                                GST_FORMAT_TIME,
                                GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
                                position
                                );
        user_data->desired_position = GST_CLOCK_TIME_NONE;
    }
}

void rct_gst_seek(RctGstUserData *user_data, gint64 position) {
    
    position = position * GST_MSECOND;
    
    if (user_data->current_state >= GST_STATE_PAUSED) {
        execute_seek(user_data, position);
    } else {
        user_data->desired_position = position;
    }
}

// Setters
void rct_gst_set_uri(RctGstUserData* user_data, gchar *_uri)
{
    GstState current_state;
    
    // Update URI if it changed
    if (g_strcmp0(user_data->configuration->uri, _uri) != 0) {
        user_data->configuration->uri = _uri;
        
        g_print("RCTGstPlayer : rct_gst_set_uri : %s\n", user_data->configuration->uri);
        
        rct_gst_set_playbin_state(user_data, GST_STATE_READY);
        
        gst_element_get_state(user_data->playbin, &current_state, NULL, 0);
        if (current_state == GST_STATE_READY) {
            rct_gst_apply_uri(user_data);
        } else {
            user_data->must_apply_uri = TRUE;
        }
    }
}

void rct_gst_set_ui_refresh_rate(RctGstUserData* user_data, guint64 uiRefreshRate)
{
    if (user_data->configuration->uiRefreshRate != uiRefreshRate) {
        user_data->configuration->uiRefreshRate = uiRefreshRate;
        g_object_set(user_data->audio_level_analyser, "interval", uiRefreshRate * 1000000, NULL);
        
        if (user_data->timeout_source) {
            g_source_destroy(user_data->timeout_source);
        }
        
        // Progression callback
        if (user_data->configuration->onPlayingProgress) {
            user_data->timeout_source = g_timeout_source_new(user_data->configuration->uiRefreshRate);
            g_source_set_callback(user_data->timeout_source, (GSourceFunc)cb_duration_and_progress, user_data, NULL);
            g_source_attach(user_data->timeout_source, NULL);
            g_source_unref(user_data->timeout_source);
        }
    }
}

void rct_gst_set_volume(RctGstUserData* user_data, gdouble volume)
{
    if (user_data->configuration->volume != volume) {
        user_data->configuration->volume = volume;
        g_object_set(user_data->playbin, "volume", user_data->configuration->volume, NULL);
    }
}

void rct_gst_set_drawable_surface(RctGstUserData *user_data, guintptr drawable_surface)
{
    if (user_data->configuration->drawableSurface != drawable_surface) {
        user_data->configuration->drawableSurface = drawable_surface;
        gst_video_overlay_prepare_window_handle(user_data->video_overlay);
    }
}
