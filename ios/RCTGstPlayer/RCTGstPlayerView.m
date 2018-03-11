//
//  EaglUiView.m
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import "RCTGstPlayerView.h"

@implementation RCTGstPlayerView

static gboolean shareInstance;
static RCTGstPlayerView *instance;

// GL surface
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

+ (RCTGstPlayerView *)getView
{
    if (shareInstance) {
        if (!instance)
            instance = [[RCTGstPlayerView alloc] init];
    } else {
        instance = [[RCTGstPlayerView alloc] init];
    }
    
    NSLog(@"Returning View : %@", instance);
    return instance;
}

// Constructor
- (instancetype)init
{
    self = [super init];
    if (self) {
        self->pipelineState = NULL;
    }
    
    return self;
}

// Callables
void onPlayerInit()
{
    // Apply what has been stored in instance
    rct_gst_set_uri([instance getUserData], [instance getUserData]->configuration->uri);
    rct_gst_set_ui_refresh_rate([instance getUserData], [instance getUserData]->configuration->uiRefreshRate);
    rct_gst_set_connect_timeout([instance getUserData], [instance getUserData]->configuration->connectTimeout);
    rct_gst_set_volume([instance getUserData], [instance getUserData]->configuration->volume);
    
    
    if (instance->pipelineState != NULL)
        [instance setPipelineState:instance->pipelineState];
    
    rct_gst_set_drawable_surface([instance getUserData], [instance getHandle]);
    
    instance.onPlayerInit(@{});
}

void onEOS()
{
    instance.onEOS(@{});
}

void onUriChanged(gchar* newUri) {
    NSString *uri = [NSString stringWithUTF8String:newUri];
    NSLog(@"RCTGstPlayer : URI : %@ - LENGTH : %d", uri, uri.length);
    instance.onUriChanged(@{ @"new_uri": uri });
}

void onPlayingProgress(gint64 progress, gint64 duration) {
    instance.onPlayingProgress(@{
                               @"progress": [NSNumber numberWithInteger:progress],
                               @"duration": [NSNumber numberWithInteger:duration]
                               });
}

void onBufferingProgress(gint progress) {
    instance.onBufferingProgress(@{
                                 @"progress": [NSNumber numberWithInteger:progress]
                                 });
}

void onElementError(gchar *source, gchar *message, gchar *debug_info) {
    instance.onElementError(@{
                            @"source": [NSString stringWithUTF8String:source],
                            @"message": [NSString stringWithUTF8String:message],
                            @"debug_info": [NSString stringWithUTF8String:debug_info]
                            });
}

void onStateChanged(GstState old_state, GstState new_state) {
    
    NSNumber* oldState = [NSNumber numberWithInt:old_state];
    NSNumber* newState = [NSNumber numberWithInt:new_state];

    instance.onStateChanged(@{ @"old_state": oldState, @"new_state": newState });
}

void onVolumeChanged(RctGstAudioLevel* audioLevel, gint nb_channels) {
    NSMutableDictionary *js_dictionary = [[NSMutableDictionary alloc] init];
    
    for (int i = 0; i < nb_channels; i++) {
        RctGstAudioLevel *audioChannelLevel = &audioLevel[i];
        [js_dictionary setObject:@{
                                   @"decay": @(audioChannelLevel->decay),
                                   @"rms": @(audioChannelLevel->rms),
                                   @"peak": @(audioChannelLevel->peak),
                                   } forKey: [NSString stringWithFormat:@"%d", i]
         ];
    }
    instance.onVolumeChanged(js_dictionary);
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Preparing configuration
        [self getUserData]->configuration->onPlayerInit = onPlayerInit;
        [self getUserData]->configuration->onEOS = onEOS;
        [self getUserData]->configuration->onUriChanged = onUriChanged;
        [self getUserData]->configuration->onPlayingProgress = onPlayingProgress;
        [self getUserData]->configuration->onBufferingProgress = onBufferingProgress;
        [self getUserData]->configuration->onElementError = onElementError;
        [self getUserData]->configuration->onStateChanged = onStateChanged;
        [self getUserData]->configuration->onVolumeChanged = onVolumeChanged;
    }
    return self;
}

-(void)layoutSubviews
{
    if (![self isReady]) {
        
        // Preparing pipeline
        rct_gst_init([self getUserData]);

        // Run it
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            rct_gst_run_loop([self getUserData]);
        });
    }
    
    [super layoutSubviews];
}

-(void)removeFromSuperview {
    [super removeFromSuperview];
    
    if (!shareInstance) {
        NSLog(@"Removing video surface : %@", self);
        rct_gst_terminate([self getUserData]);
    }
}

// Getters
- (guintptr)getHandle
{
    return (guintptr)(id)self;
}

- (RctGstUserData *)getUserData
{
    if (!self->userData) {
        NSLog(@"Creating user data");
        self->userData = rct_gst_init_user_data();
    }
    return self->userData;
}

- (gboolean)isReady
{
    return [self getUserData]->is_ready;
}

// Methods
- (void)setPipelineState:(int)pipelineState {
    self->pipelineState = pipelineState;
    rct_gst_set_playbin_state([self getUserData], self->pipelineState);
}

- (void)seek:(gint64)position {
    rct_gst_seek([self getUserData], position);
}

// Setters
- (void)setShareInstance:(BOOL)_shareInstance {
    shareInstance = _shareInstance;
}
@end

