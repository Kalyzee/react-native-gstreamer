//
//  gstreamer_backend.c
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#include "gstreamer_backend.h"
#include <stdlib.h>

// Log info
GST_DEBUG_CATEGORY_STATIC(rct_gst_player);

// Setters
void rct_gst_set_uri(RctGstUserData* user_data, gchar* _uri)
{
    user_data->configuration->uri = _uri;
    if (user_data->pipeline && !user_data->configuration->isDebugging)
        rct_gst_apply_uri(user_data);
}

void rct_gst_set_audio_level_refresh_rate(RctGstUserData* user_data, gint audio_level_refresh_rate)
{
    user_data->configuration->audioLevelRefreshRate = audio_level_refresh_rate;
    g_object_set(user_data->audio_level_element, "interval", audio_level_refresh_rate * 1000000, NULL);
}

void rct_gst_set_debugging(RctGstUserData* user_data, gboolean is_debugging)
{
    user_data->configuration->isDebugging = is_debugging;
    // TODO : Recreate pipeline...
}

void rct_gst_set_volume(RctGstUserData* user_data, gdouble volume)
{
    user_data->configuration->volume = volume;
    g_print("New volume : %f for %p", user_data->configuration->volume, user_data->audio_sink);
    g_object_set(user_data->volume_element, "volume", user_data->configuration->volume, NULL);
}

// User data
RctGstUserData *rct_gst_init_user_data()
{
    // Create user data structure
    RctGstUserData *user_data = calloc(1, sizeof(RctGstUserData));
    
    // Create configuration structure
    user_data->configuration = calloc(1, sizeof(RctGstConfiguration));
    user_data->configuration->uri = calloc(1, sizeof(gchar));
    user_data->configuration->audioLevelRefreshRate = 100;
    user_data->configuration->isDebugging = FALSE;
    
    // Create audio level structure
    user_data->audio_level = calloc(1, sizeof(RctGstAudioLevel));
    
    return user_data;
}

void rct_gst_free_user_data(RctGstUserData *user_data)
{
    free(user_data->configuration->uri);
    
    free(user_data->configuration);
    free(user_data->audio_level);
    free(user_data);
}

/**********************
 VIDEO HANDLING METHODS
 *********************/
void rct_gst_set_drawable_surface(RctGstUserData* user_data, guintptr _drawableSurface)
{
    user_data->configuration->drawableSurface = _drawableSurface;
    rct_gst_apply_drawable_surface(user_data);
}

/**********************
 AUDIO HANDLING METHODS
 *********************/
GstElement* create_audio_sink(RctGstUserData* user_data)
{
    // New audio bin
    GstElement *leveledsink = gst_bin_new("leveledsink");
    
    // Create an audio level analyzing filter with 100ms refresh rate
    user_data->audio_level_element = gst_element_factory_make("level", NULL);
    
    // Creating audio sink
    user_data->audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    gst_bin_add_many(GST_BIN(leveledsink), user_data->audio_level_element, user_data->audio_sink, NULL);
    
    // Linking them
    if(!gst_element_link(user_data->audio_level_element, user_data->audio_sink))
        g_printerr("Failed to link audio_level and audio_sink");
    
    // Creating pad and ghost pad
    GstPad *levelPad = gst_element_get_static_pad(user_data->audio_level_element, "sink");
    gst_element_add_pad(leveledsink, gst_ghost_pad_new("sink", levelPad));
    gst_object_unref(GST_OBJECT(levelPad));
    
    return leveledsink;
}

GstBusSyncReply cb_create_window(GstBus *bus, GstMessage *message, RctGstUserData* user_data)
{
    if(!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;
    
    g_print("cb_create_window\n");
    if (user_data->video_overlay) {
        g_print("cb_create_window -> ok\n");
        gst_video_overlay_set_window_handle(user_data->video_overlay, user_data->configuration->drawableSurface);
    } else {
        g_print("cb_create_window -> nok : overlay -> %p, surface -> %p\n", user_data->video_overlay, user_data->configuration->drawableSurface);
    }
    
    gst_message_unref(message);
    return GST_BUS_DROP;
}

/*********************
 APPLICATION CALLBACKS
 ********************/
static void cb_error(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    GError *err;
    gchar *debug_info;
    
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    if (user_data->configuration->onElementError) {
        user_data->configuration->onElementError(GST_OBJECT_NAME(msg->src), err->message, debug_info);
    }
    g_clear_error(&err);
    g_free(debug_info);
    rct_gst_set_pipeline_state(user_data, GST_STATE_NULL);
}

static void cb_eos(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    if (user_data->configuration->onEOS) {
        user_data->configuration->onEOS();
    }
}

static void cb_state_changed(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    
    // Only pay attention to messages coming from the pipeline, not its children
    if(GST_MESSAGE_SRC(msg) == GST_OBJECT(user_data->pipeline))
    {
        if (user_data->configuration->onStateChanged) {
            user_data->configuration->onStateChanged(old_state, new_state);
        }
    }
    
}

static gboolean cb_message_element(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    if(msg->type == GST_MESSAGE_ELEMENT)
    {
        const GstStructure *s = gst_message_get_structure(msg);
        const gchar *name = gst_structure_get_name(s);
        
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
            
            // RMS
            value = g_value_array_get_nth(rms_arr, 0);
            rms_dB = g_value_get_double(value);
            user_data->audio_level->rms = pow(10, rms_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // PEAK
            value = g_value_array_get_nth(peak_arr, 0);
            peak_dB = g_value_get_double(value);
            user_data->audio_level->peak = pow(10, peak_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // DECAY
            value = g_value_array_get_nth(decay_arr, 0);
            decay_dB = g_value_get_double(value);
            user_data->audio_level->decay = pow(10, decay_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            if (user_data->configuration->onVolumeChanged) {
                user_data->configuration->onVolumeChanged(user_data->audio_level);
            }
        }
    }
    return TRUE;
}

// Remove latency as much as possible
static void cb_setup_source(GstElement *pipeline, GstElement *source, RctGstUserData* user_data) {
    g_object_set (source, "latency", 0, NULL);
}

static gboolean cb_async_done(GstBus *bus, GstMessage *message, RctGstUserData* user_data)
{
    return TRUE;
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
            
        case GST_MESSAGE_ASYNC_DONE:
            cb_async_done(bus, message, user_data);
            break;
            
        default:
            break;
    }
    
    return TRUE;
}

/*************
 OTHER METHODS
 ************/
GstStateChangeReturn rct_gst_set_pipeline_state(RctGstUserData* user_data, GstState state)
{
    GstState currentState;
    GstStateChangeReturn validity;
    
    gst_element_get_state(user_data->pipeline, &currentState, NULL, 0);
    if (state != currentState) {
        g_print("Pipeline state requested : %s\n", gst_element_state_get_name(state));
        validity = gst_element_set_state(user_data->pipeline, state);
    }
    return validity;
}

void rct_gst_init(RctGstUserData* user_data)
{
    gchar *launch_command;
    
    user_data->main_loop = g_main_loop_new(NULL, FALSE);
    
    // Prepare playbin pipeline. If playbin not working, will display an error video signal
    launch_command = (!user_data->configuration->isDebugging) ? "playbin" : "videotestsrc ! glimagesink name=video-sink";
    user_data->pipeline = gst_parse_launch(launch_command, NULL);
    
    // Remove latency as much as possible
    if (!user_data->configuration->isDebugging) {
        g_signal_connect(G_OBJECT(user_data->pipeline), "source-setup", (GCallback) cb_setup_source, user_data);
    }
    
    // Preparing bus
    user_data->bus = gst_element_get_bus(user_data->pipeline);
    user_data->bus_watch_id = gst_bus_add_watch(user_data->bus, cb_bus_watch, user_data);
    
    // First time, need a surface to draw on - then use rct_gst_set_drawable_surface
    gst_bus_set_sync_handler(user_data->bus,(GstBusSyncHandler)cb_create_window, user_data, NULL);
    gst_object_unref(user_data->bus);
    
    // Change audio sink with a custom one (Allow volume analysis)
    user_data->audio_sink = create_audio_sink(user_data);

    // Create necessary elements to have volume analysis and sound test while debugging
    if (user_data->configuration->isDebugging) {
        user_data->audiotestsrc = gst_element_factory_make("audiotestsrc", "audiotestsrc");
        user_data->volume_element = user_data->audiotestsrc;
        gst_bin_add_many(GST_BIN(user_data->pipeline), user_data->audiotestsrc, user_data->audio_sink, NULL);
        gst_element_link(user_data->audiotestsrc, user_data->audio_sink);
    
    // Create necessary elements to have volume analysis on playbin
    } else {
        g_object_set(user_data->pipeline, "audio-sink", user_data->audio_sink, NULL);
        user_data->volume_element = user_data->pipeline;
    }
    
    // Store video sink global items
    if (user_data->configuration->isDebugging) {
        user_data->video_sink = gst_bin_get_by_name(user_data->pipeline, "video-sink");
    } else {
        user_data->video_sink = gst_element_factory_make("glimagesink", "video-sink");
        g_object_set(GST_OBJECT(user_data->pipeline), "video-sink", user_data->video_sink, NULL);
    }
    
    user_data->video_overlay = GST_VIDEO_OVERLAY(user_data->video_sink);
    
    // Apply URI
    if (!user_data->configuration->isDebugging && user_data->configuration->uri && g_strcmp0("", user_data->configuration->uri) != 0)
        rct_gst_apply_uri(user_data);
    
    // Apply Drawable surface
    if (user_data->pipeline != NULL & user_data->configuration->drawableSurface != NULL)
        rct_gst_apply_drawable_surface(user_data);
}

void rct_gst_run_loop(RctGstUserData* user_data)
{
    if (user_data->configuration->onInit) {
        user_data->configuration->onInit();
    }
    
    g_main_loop_run(user_data->main_loop);
    g_main_loop_unref(user_data->main_loop);
    
    gst_element_set_state (user_data->pipeline, GST_STATE_NULL);
    gst_object_unref(user_data->pipeline);
    
    gst_object_unref(user_data->video_sink);
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

void rct_gst_apply_drawable_surface(RctGstUserData* user_data)
{
    gst_video_overlay_prepare_window_handle(user_data->video_overlay);
}

void rct_gst_apply_uri(RctGstUserData* user_data)
{
    rct_gst_set_pipeline_state(user_data, GST_STATE_READY);
    g_object_set(user_data->pipeline, "uri", user_data->configuration->uri, NULL);
    g_print("URI : %s\n", user_data->configuration->uri);
    if (user_data->configuration->onUriChanged) {
        user_data->configuration->onUriChanged(user_data->configuration->uri);
    }
}
