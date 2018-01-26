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
    if (user_data->pipeline)
        rct_gst_set_pipeline_state(user_data, GST_STATE_READY);
}

void rct_gst_set_audio_level_refresh_rate(RctGstUserData* user_data, gint audio_level_refresh_rate)
{
    user_data->configuration->audioLevelRefreshRate = audio_level_refresh_rate;
    g_object_set(user_data->audio_level_analyser, "interval", audio_level_refresh_rate * 1000000, NULL);
}

void rct_gst_set_debugging(RctGstUserData* user_data, gboolean is_debugging)
{
    user_data->configuration->isDebugging = is_debugging;
    
    GstState currentState = GST_STATE_PLAYING;
    
    if (user_data->configuration->isDebugging) {
        g_object_set(user_data->audio_selector, "active-pad", user_data->audio_selector_debug_sink_pad, NULL);
        g_object_set(user_data->video_selector, "active-pad", user_data->video_selector_debug_sink_pad, NULL);
    } else {
        g_object_set(user_data->audio_selector, "active-pad", user_data->audio_selector_sink_pad, NULL);
        g_object_set(user_data->video_selector, "active-pad", user_data->video_selector_sink_pad, NULL);
    }
    
    gst_element_get_state(user_data->pipeline, &currentState, NULL, 10);

    if (currentState != GST_STATE_PLAYING)
        gst_element_set_state(user_data->pipeline, GST_STATE_PLAYING);
}

void rct_gst_set_volume(RctGstUserData* user_data, gdouble volume)
{
    g_object_set(user_data->audio_test_src, "volume", user_data->configuration->volume, NULL);
    g_object_set(user_data->audio_volume, "volume", user_data->configuration->volume, NULL);
}

// User data
RctGstUserData *rct_gst_init_user_data()
{
    // Create user data structure
    RctGstUserData *user_data = calloc(1, sizeof(RctGstUserData));
    
    // Create configuration structure
    user_data->configuration = calloc(1, sizeof(RctGstConfiguration));
    user_data->configuration->uri = "";
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
void create_video_sink_bin(RctGstUserData *user_data)
{
    // Create elements
    user_data->video_sink_bin = gst_bin_new("video-sink-bin");
    user_data->video_test_src = gst_element_factory_make("videotestsrc", "video-test-src");
    user_data->video_selector = gst_element_factory_make("input-selector", "video-selector");
    user_data->video_convertor = gst_element_factory_make("videoconvert", "video-convertor");
    user_data->video_sink = gst_element_factory_make("glimagesink", "video-sink");
    user_data->video_overlay = GST_VIDEO_OVERLAY(user_data->video_sink);

    // Add them
    gst_bin_add_many(GST_BIN(user_data->video_sink_bin),
                     user_data->video_test_src,
                     user_data->video_selector,
                     user_data->video_convertor,
                     user_data->video_sink,
                     NULL);
    
    // Creating video-selector pads (on request)
    user_data->video_selector_sink_pad = gst_element_get_request_pad(user_data->video_selector, "sink_%u");
    user_data->video_selector_debug_sink_pad = gst_element_get_request_pad(user_data->video_selector, "sink_%u");
    
    // Link them (Except video-selector sink pads - not yet)
    if(!gst_pad_link(gst_element_get_static_pad(user_data->video_test_src, "src"), user_data->video_selector_debug_sink_pad) == GST_PAD_LINK_OK)
        g_printerr("Failed to link video-test-src and video-selector\n");
    
    if(!gst_element_link(user_data->video_selector, user_data->video_convertor))
        g_printerr("Failed to link video-selector and video-convertor\n");
    
    if(!gst_element_link(user_data->video_convertor, user_data->video_sink))
        g_printerr("Failed to link video-convertor and gl-image-sink\n");
    
    // Creating ghostpad for playbin (needs a pad on sink named "sink")
    gst_element_add_pad(GST_BIN(user_data->video_sink_bin), gst_ghost_pad_new("sink", user_data->video_selector_sink_pad));
}

/**********************
 AUDIO HANDLING METHODS
 *********************/
void create_audio_sink_bin(RctGstUserData *user_data)
{
    GstElement *audio_sink;
    
    // Create elements
    user_data->audio_sink_bin = gst_bin_new("audio-sink-bin");
    user_data->audio_test_src = gst_element_factory_make("audiotestsrc", "audio-test-src");
    user_data->audio_selector = gst_element_factory_make("input-selector", "audio-selector");
    user_data->audio_convertor = gst_element_factory_make("audioconvert", "audio-convertor");
    user_data->audio_volume = gst_element_factory_make("volume", "audio-volume");
    user_data->audio_level_analyser = gst_element_factory_make("level", "audio-level-analyser");
    audio_sink = gst_element_factory_make("autoaudiosink", "audio-sink");
    
    // Add them
    gst_bin_add_many(GST_BIN(user_data->audio_sink_bin),
                     user_data->audio_test_src,
                     user_data->audio_selector,
                     user_data->audio_convertor,
                     user_data->audio_volume,
                     user_data->audio_level_analyser,
                     audio_sink,
                     NULL);
    
    // Creating audio-selector pads (on request)
    user_data->audio_selector_sink_pad = gst_element_get_request_pad(user_data->audio_selector, "sink_%u");
    user_data->audio_selector_debug_sink_pad = gst_element_get_request_pad(user_data->audio_selector, "sink_%u");
    
    // Link them (Except audio-selector sink pads - not yet)
    if(!gst_pad_link(gst_element_get_static_pad(user_data->audio_test_src, "src"), user_data->audio_selector_debug_sink_pad) == GST_PAD_LINK_OK)
        g_printerr("Failed to link audio-test-src and audio-selector\n");

    if(!gst_element_link(user_data->audio_selector, user_data->audio_convertor))
        g_printerr("Failed to link audio-selector-selector and audio-convertor\n");
    
    if(!gst_element_link(user_data->audio_convertor, user_data->audio_volume))
        g_printerr("Failed to link audio-convertor and audio-volume\n");
    
    if(!gst_element_link(user_data->audio_volume, user_data->audio_level_analyser))
        g_printerr("Failed to link audio-volume and audio-level-analyser\n");
    
    if(!gst_element_link(user_data->audio_level_analyser, audio_sink))
        g_printerr("Failed to link audio-level-analyser and audio-sink\n");
    
    // Creating ghostpad for playbin (needs a pad on sink named "sink")
    gst_element_add_pad(GST_BIN(user_data->audio_sink_bin), gst_ghost_pad_new("sink", user_data->audio_selector_sink_pad));
}

/*********************
 APPLICATION CALLBACKS
 ********************/

static void cb_decode_bin_pad_added(GstElement *decodebin, GstPad *pad, RctGstUserData* user_data)
{
    GstCaps *caps = gst_pad_query_caps(pad, NULL);
    GstStructure *str = gst_caps_get_structure(caps, 0);
    GstPad *audioPad = gst_element_get_static_pad(user_data->audio_sink_bin, "sink");
    GstPad *videoPad = gst_element_get_static_pad(user_data->video_sink_bin, "sink");
    
    GstPadLinkReturn returned_link_status;
    
    g_print("cb_decode_bin_pad_added : %s\n", gst_structure_get_name(str));

    if (g_strrstr(gst_structure_get_name(str), "audio")) {
        g_print("cb_decode_bin_pad_added : Linking audio\n");
        returned_link_status = gst_pad_link(pad, audioPad);
        if (!returned_link_status == GST_PAD_LINK_OK)
            g_printerr("Unable to link audio pad : %s\n", gst_pad_link_get_name(returned_link_status));
    }
    
    if (g_strrstr(gst_structure_get_name(str), "video")) {
        g_print("cb_decode_bin_pad_added : Linking video\n");
        returned_link_status = gst_pad_link(pad, videoPad);
        if (!returned_link_status == GST_PAD_LINK_OK)
            g_printerr("Unable to link video pad : %s\n", gst_pad_link_get_name(returned_link_status));
    }
    
    gst_caps_unref(caps);
    g_object_unref(audioPad);
    g_object_unref(videoPad);
}

GstBusSyncReply cb_create_window(GstBus *bus, GstMessage *message, RctGstUserData* user_data)
{
    if(!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;
    
    if (user_data->video_overlay) {
        g_print("cb_create_window -> ok\n");
        gst_video_overlay_set_window_handle(user_data->video_overlay, user_data->configuration->drawableSurface);
    } else {
        g_print("cb_create_window -> nok : overlay -> %p, surface -> %p\n", user_data->video_overlay, user_data->configuration->drawableSurface);
    }
    
    gst_message_unref(message);
    return GST_BUS_DROP;
}

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
}

static void cb_eos(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    if (!user_data->configuration->isDebugging)
        rct_gst_set_pipeline_state(user_data, GST_STATE_READY);
    
    g_print("EOS recieved\n");
    
    if (user_data->configuration->onEOS) {
        user_data->configuration->onEOS();
    }
}

static void cb_state_changed(GstBus *bus, GstMessage *msg, RctGstUserData* user_data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    
    // Pay attention to messages coming from the pipeline, not its children
    if(GST_MESSAGE_SRC(msg) == GST_OBJECT(user_data->pipeline))
    {
        g_print("New state %s\n", gst_element_state_get_name(new_state));
        
        // We apply the uri when the pipeline is in ready state
        if (new_state == GST_STATE_READY) {
            rct_gst_apply_uri(user_data);
        }

        if (new_state >= GST_STATE_READY) {
            rct_gst_set_volume(user_data, user_data->configuration->volume);
        }

        if (new_state == GST_STATE_READY && old_state == GST_STATE_PAUSED) {
            if (user_data->configuration->onStateReadyToPause) {
                user_data->configuration->onStateReadyToPause();
            }
        }

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
            user_data->audio_level->rms = rms_dB; //pow(10, rms_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // PEAK
            value = g_value_array_get_nth(peak_arr, 0);
            peak_dB = g_value_get_double(value);
            user_data->audio_level->peak = peak_dB; //pow(10, peak_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // DECAY
            value = g_value_array_get_nth(decay_arr, 0);
            decay_dB = g_value_get_double(value);
            user_data->audio_level->decay = decay_dB; // pow(10, decay_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            if (user_data->configuration->onVolumeChanged) {
                user_data->configuration->onVolumeChanged(user_data->audio_level);
            }
        }
    }
    return TRUE;
}

// Remove latency as much as possible
static void cb_setup_source(GstElement *pipeline, GstElement *source, RctGstUserData* user_data) {
    g_object_set(source, "latency", 0, NULL);
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
        validity = gst_element_set_state(user_data->pipeline, state);
        g_print(
                "Pipeline state requested : %s. Result : %s\n",
                gst_element_state_get_name(state),
                gst_element_state_change_return_get_name(validity)
                );
        
    }
    return validity;
}

void rct_gst_init(RctGstUserData* user_data)
{
    gchar *launch_command;
    
    user_data->main_loop = g_main_loop_new(NULL, FALSE);
    
    // Create pipeline
    user_data->pipeline = gst_pipeline_new("pipeline");
    
    // Preparing bus
    user_data->bus = gst_element_get_bus(user_data->pipeline);
    user_data->bus_watch_id = gst_bus_add_watch(user_data->bus, cb_bus_watch, user_data);

    gst_bus_set_sync_handler(user_data->bus,(GstBusSyncHandler)cb_create_window, user_data, NULL);
    gst_object_unref(user_data->bus);
    
    // (SINKS) - Create audio-sink-bin and video-sink-bin
    create_audio_sink_bin(user_data);
    create_video_sink_bin(user_data);
    
    // Create uri-decode-bin
    user_data->uri_decode_bin = gst_element_factory_make("uridecodebin", "uri-decode-bin");
    
    // Connect uri-decode-bin signals
    g_signal_connect(G_OBJECT(user_data->uri_decode_bin), "source-setup", (GCallback) cb_setup_source, user_data);
    g_signal_connect(G_OBJECT(user_data->uri_decode_bin), "pad-added", G_CALLBACK (cb_decode_bin_pad_added), user_data);
    
    // Add all elements to the pipeline
    gst_bin_add_many(user_data->pipeline, user_data->uri_decode_bin, user_data->audio_sink_bin, user_data->video_sink_bin, NULL);
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

void rct_gst_apply_uri(RctGstUserData* user_data)
{
    // Get current URI
    gchar *current_uri;
    
    g_object_get(user_data->uri_decode_bin, "uri", &current_uri, NULL);
    
    // Change URI if different
    if (g_strcmp0(current_uri, user_data->configuration->uri) != 0) {
        // g_object_set(user_data->uri_decode_bin, "uri", user_data->configuration->uri, NULL);
        
        if (user_data->configuration->onUriChanged) {
            user_data->configuration->onUriChanged(user_data->configuration->uri);
        }
    }
    
    // Clear resources
    g_free(current_uri);
}
