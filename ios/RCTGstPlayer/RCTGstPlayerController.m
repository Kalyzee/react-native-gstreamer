//
//  RCTGstPlayerController.m
//  RCTGstPlayer
//
//  Created by Alann on 20/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import "RCTGstPlayerController.h"

@interface RCTGstPlayerController ()
{
    RctGstConfiguration *configuration;
    BOOL isInited;
}

@end

@implementation RCTGstPlayerController

// For access in pure C callbacks
EaglUIView *c_view;

gchar *new_uri;
gchar *source, *message, *debug_info;

RctGstAudioLevel* audioLevel;

NSNumber* oldState;
NSNumber* newState;

// Generate custom view to return to react-native (for events handle)
@dynamic view;
- (void)loadView {
    
    c_view = [[EaglUIView alloc] init];
    [c_view setBackgroundColor:[UIColor redColor]];
    
    self.view = c_view;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        new_uri = g_malloc(sizeof(gchar));
        source = g_malloc(sizeof(gchar));
        message = g_malloc(sizeof(gchar));
        debug_info = g_malloc(sizeof(gchar));
        audioLevel = g_malloc(sizeof(RctGstAudioLevel));
    }
    return self;
}

// When the player has inited
void onInit() {
    if (c_view.onPlayerInit)
        c_view.onPlayerInit(@{});
}

void onStateChanged(GstState old_state, GstState new_state) {
    
    oldState = [NSNumber numberWithInt:old_state];
    newState = [NSNumber numberWithInt:new_state];
    
    if (c_view.onStateChanged)
        c_view.onStateChanged(@{ @"old_state": oldState, @"new_state": newState });
}

void onVolumeChanged(RctGstAudioLevel* _audioLevel) {
    
    audioLevel->decay = _audioLevel->decay;
    audioLevel->peak = _audioLevel->peak;
    audioLevel->rms = _audioLevel->rms;
    
    if (onVolumeChanged)
        c_view.onVolumeChanged(@{
                                 @"decay": @(audioLevel->decay),
                                 @"rms": @(audioLevel->rms),
                                 @"peak": @(audioLevel->peak),
                                 });
}

void onUriChanged(gchar* newUri) {
    new_uri = g_strdup(newUri);
    
    NSString *uri = [NSString stringWithUTF8String:new_uri];
    NSLog(@"RCTGstPlayer : URI : %@ - LENGTH : %d", uri, uri.length);
    if (c_view.onUriChanged)
        c_view.onUriChanged(@{ @"new_uri": uri });
}

void onEOS() {
    
    c_view.onEOS(@{});
}

void onElementError(gchar *_source, gchar *_message, gchar *_debug_info) {
    source = g_strdup(_source);
    message = g_strdup(_message);
    debug_info = g_strdup(_debug_info);
    
    if (c_view.onElementError)
        c_view.onElementError(@{
                                @"source": [NSString stringWithUTF8String:source],
                                @"message": [NSString stringWithUTF8String:message],
                                @"debug_info": [NSString stringWithUTF8String:debug_info]
                                });
}

- (void)initGst {
    // Preparing configuration
    configuration = rct_gst_get_configuration();
    
    configuration->onInit = onInit;
    configuration->onStateChanged = onStateChanged;
    configuration->onVolumeChanged = onVolumeChanged;
    configuration->onUriChanged = onUriChanged;
    configuration->onEOS = onEOS;
    configuration->onElementError = onElementError;
    
    // Preparing pipeline
    rct_gst_init();

    self->isInited = TRUE;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    if (!self->isInited)
        [self initGst];
    
    // Run pipeline
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        rct_gst_set_drawable_surface([c_view getHandle]);
        rct_gst_run_loop();
    });
}

// Memory management
- (void)dealloc
{
    NSLog(@"Deallocate RCTGstPlayer");
    
    rct_gst_terminate();
    
    g_free(new_uri);
    g_free(source);
    g_free(message);
    g_free(debug_info);
    g_free(audioLevel);
}

@end
