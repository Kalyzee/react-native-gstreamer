//
//  gstreamer_backend.c
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#include "gstreamer_backend.h"

// Log info
GST_DEBUG_CATEGORY_STATIC(rct_gst_player);

// Globals items
RctGstAudioLevel* audio_level;
RctGstConfiguration* configuration;

GstElement *pipeline;
GMainLoop *main_loop;
guint bus_watch_id;
GstBus *bus;

// Video
GstVideoOverlay* video_overlay;

// Audio
GstElement* audio_level_element;

// Sinks
GstElement *video_sink;
GstElement *audio_sink;

// Getters
RctGstConfiguration *rct_gst_get_configuration()
{
    if (!configuration) {
        configuration = g_malloc(sizeof(RctGstConfiguration));
        configuration->audioLevelRefreshRate = 100;
        configuration->uri = NULL;
        configuration->isDebugging = FALSE;
        
        configuration->onElementError = NULL;
        configuration->onStateChanged = NULL;
        configuration->onVolumeChanged = NULL;
        configuration->onUriChanged = NULL;
        
        configuration->onInit = NULL;
        configuration->onEOS = NULL;
        configuration->drawableSurface = NULL;
    }
    return configuration;
}

RctGstAudioLevel *rct_gst_get_audio_level()
{
    if (!audio_level) {
        audio_level = g_malloc(sizeof(RctGstAudioLevel));
    }
    return audio_level;
}

// Setters
void rct_gst_set_uri(gchar* _uri)
{
    rct_gst_get_configuration()->uri = _uri;
    if (pipeline)
        rct_gst_apply_uri();
}

void rct_gst_set_audio_level_refresh_rate(gint audio_level_refresh_rate)
{
    rct_gst_get_configuration()->audioLevelRefreshRate = audio_level_refresh_rate;
    // g_object_set(audio_level_element, "interval", audio_level_refresh_rate * 1000000, NULL);
}

void rct_gst_set_debugging(gboolean is_debugging)
{
    rct_gst_get_configuration()->isDebugging = is_debugging;
    // TODO : Recreate pipeline...
}

/**********************
 VIDEO HANDLING METHODS
 *********************/
void rct_gst_set_drawable_surface(guintptr _drawableSurface)
{
    g_print("rct_gst_set_drawable_surface\n");
    rct_gst_get_configuration()->drawableSurface = _drawableSurface;
    rct_gst_apply_drawable_surface();
}

/**********************
 AUDIO HANDLING METHODS
 *********************/
GstElement* create_audio_sink()
{
    // Prepare audio level structure
    rct_gst_get_audio_level();
    
    // New audio bin
    GstElement *leveledsink = gst_bin_new("leveledsink");
    
    // Create an audio level analyzing filter with 100ms refresh rate
    audio_level_element = gst_element_factory_make("level", NULL);
    
    // Creating audio sink
    GstElement *audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    gst_bin_add_many(GST_BIN(leveledsink), audio_level_element, audio_sink, NULL);
    
    // Linking them
    if(!gst_element_link(audio_level_element, audio_sink))
        g_printerr("Failed to link audio_level and audio_sink");
    
    // Creating pad and ghost pad
    GstPad *levelPad = gst_element_get_static_pad(audio_level_element, "sink");
    gst_element_add_pad(leveledsink, gst_ghost_pad_new("sink", levelPad));
    gst_object_unref(GST_OBJECT(levelPad));
    
    return leveledsink;
}

GstBusSyncReply cb_create_window(GstBus *bus, GstMessage *message, gpointer user_data)
{
    if(!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;
    
    g_print("cb_create_window\n");
    if (video_overlay && rct_gst_get_configuration()->drawableSurface) {
        g_print("cb_create_window -> ok\n");
        gst_video_overlay_set_window_handle(video_overlay, rct_gst_get_configuration()->drawableSurface);
    } else {
        g_print("cb_create_window -> nok : overlay -> %p, surface -> %p\n", video_overlay, rct_gst_get_configuration()->drawableSurface);
    }
    
    gst_message_unref(message);
    return GST_BUS_DROP;
}

/*********************
 APPLICATION CALLBACKS
 ********************/
static void cb_error(GstBus *bus, GstMessage *msg, gpointer *user_data)
{
    GError *err;
    gchar *debug_info;
    
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    if (rct_gst_get_configuration()->onElementError) {
        rct_gst_get_configuration()->onElementError(GST_OBJECT_NAME(msg->src), err->message, debug_info);
    }
    g_clear_error(&err);
    g_free(debug_info);
    rct_gst_set_pipeline_state(GST_STATE_NULL);
}

static void cb_eos(GstBus *bus, GstMessage *msg, gpointer *user_data)
{
    if (rct_gst_get_configuration()->onEOS) {
        rct_gst_get_configuration()->onEOS();
    }
}

static void cb_state_changed(GstBus *bus, GstMessage *msg, gpointer *user_data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);

    // Only pay attention to messages coming from the pipeline, not its children
    if(GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline))
    {
        if (rct_gst_get_configuration()->onStateChanged) {
            rct_gst_get_configuration()->onStateChanged(old_state, new_state);
        }
    }
    
}

static gboolean cb_message_element(GstBus *bus, GstMessage *msg, gpointer *user_data)
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
            rct_gst_get_audio_level()->rms = pow(10, rms_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // PEAK
            value = g_value_array_get_nth(peak_arr, 0);
            peak_dB = g_value_get_double(value);
            rct_gst_get_audio_level()->peak = pow(10, peak_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            // DECAY
            value = g_value_array_get_nth(decay_arr, 0);
            decay_dB = g_value_get_double(value);
            rct_gst_get_audio_level()->decay = pow(10, decay_dB / 20); // converting from dB to normal gives us a value between 0.0 and 1.0
            
            if (rct_gst_get_configuration()->onVolumeChanged){
                rct_gst_get_configuration()->onVolumeChanged(rct_gst_get_audio_level());
            }
        }
    }
    return TRUE;
}

// Remove latency as much as possible
static void cb_setup_source(GstElement *pipeline, GstElement *source, void *data) {
    g_object_set (source, "latency", 0, NULL);
}

static gboolean cb_async_done(GstBus *bus, GstMessage *message, gpointer user_data)
{
    return TRUE;
}

static gboolean cb_bus_watch(GstBus *bus, GstMessage *message, gpointer user_data)
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
GstStateChangeReturn rct_gst_set_pipeline_state(GstState state)
{
    g_print("Pipeline state requested : %s\n", gst_element_state_get_name(state));
    GstStateChangeReturn validity = gst_element_set_state(pipeline, state);
    g_print("Validity : %s\n", gst_element_state_change_return_get_name(validity));

    return validity;
}

void rct_gst_init()
{
    gchar *launch_command;

    // Prepare playbin pipeline. If playbin not working, will display an error video signal
    launch_command = (!rct_gst_get_configuration()->isDebugging) ? "playbin" : "videotestsrc ! glimagesink name=video-sink";
    pipeline = gst_parse_launch(launch_command, NULL);
    
    // Remove latency as much as possible
    g_signal_connect(G_OBJECT(pipeline), "source-setup", (GCallback) cb_setup_source, NULL);
    
    // Preparing bus
    bus = gst_element_get_bus(pipeline);
    bus_watch_id = gst_bus_add_watch(bus, cb_bus_watch, NULL);
    
    // First time, need a surface to draw on - then use rct_gst_set_drawable_surface
    gst_bus_set_sync_handler(bus,(GstBusSyncHandler)cb_create_window, pipeline, NULL);
    gst_object_unref(bus);
    
    // Change audio sink with a custom one(Allow volume analysis)
    if (!rct_gst_get_configuration()->isDebugging) {
        audio_sink = create_audio_sink();
        g_object_set(pipeline, "audio-sink", audio_sink, NULL);
    }
    
    // Store video sink global items
    if (rct_gst_get_configuration()->isDebugging)
        video_sink = gst_bin_get_by_name(pipeline, "video-sink");
    else
        video_sink = gst_element_factory_make("glimagesink", "video-sink");

    if (!rct_gst_get_configuration()->isDebugging)
        g_object_set(GST_OBJECT(pipeline), "video-sink", video_sink, NULL);
    
    video_overlay = GST_VIDEO_OVERLAY(video_sink);
    
    g_print("Video sink : %p\n", video_sink);
    g_print("Video overlay : %p\n", video_overlay);

    // Apply URI
    if (!rct_gst_get_configuration()->isDebugging && pipeline != NULL && rct_gst_get_configuration()->uri != NULL)
        rct_gst_apply_uri();
    
    // Apply Drawable surface
    if (pipeline != NULL & rct_gst_get_configuration()->drawableSurface != NULL)
        rct_gst_apply_drawable_surface();
    
    if (rct_gst_get_configuration()->onInit) {
        rct_gst_get_configuration()->onInit();
    }
}

void rct_gst_run_loop()
{
    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);
    
    g_main_loop_unref (main_loop);
    main_loop = NULL;
    gst_element_set_state (pipeline, GST_STATE_NULL);

    if(video_sink != NULL)
        gst_object_unref(video_sink);
    
    if(video_overlay != NULL)
        gst_object_unref(video_overlay);
    
    if(rct_gst_get_configuration()->drawableSurface != NULL)
        rct_gst_get_configuration()->drawableSurface = NULL;
    
    g_source_remove(bus_watch_id);
    
    g_free(configuration);
    g_free(audio_level);

    pipeline = NULL;
    configuration = NULL;
    audio_level = NULL;
}

void rct_gst_terminate()
{
    /* Free resources */
    g_main_loop_quit(main_loop);
}

gchar *rct_gst_get_info()
{
    return gst_version_string();
}

void rct_gst_apply_drawable_surface()
{
    g_print("rct_gst_apply_drawable_surface\n");
    gst_video_overlay_prepare_window_handle(video_overlay);
}

void rct_gst_apply_uri()
{
    rct_gst_set_pipeline_state(GST_STATE_READY);
    g_object_set(pipeline, "uri", rct_gst_get_configuration()->uri, NULL);
    if (rct_gst_get_configuration()->onUriChanged) {
        rct_gst_get_configuration()->onUriChanged(rct_gst_get_configuration()->uri);
    }
}
