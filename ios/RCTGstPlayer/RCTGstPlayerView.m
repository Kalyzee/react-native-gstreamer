//
//  EaglUiView.m
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import "RCTGstPlayerView.h"

@implementation RCTGstPlayerView
@synthesize delegate;

// GL surface
+ (Class) layerClass
{
    return [CAEAGLLayer class];
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
void onPlayerInit(RCTGstPlayerView *self)
{
    // Apply what has been stored in instance
    rct_gst_set_uri([self getUserData], [self getUserData]->configuration->uri);
    rct_gst_set_ui_refresh_rate([self getUserData], [self getUserData]->configuration->uiRefreshRate);
    rct_gst_set_volume([self getUserData], [self getUserData]->configuration->volume);
    
    if (self->pipelineState != NULL)
        [self setPipelineState:self->pipelineState];
    
    rct_gst_set_drawable_surface([self getUserData], [self getHandle]);
    
    [self.delegate playerInited:self];
}

void onPadAdded(RCTGstPlayerView *self, gchar *name)
{
    if (self && self.delegate)
    {
        NSString *pad_name = [NSString stringWithUTF8String:name];
        [self.delegate padAdded:self withData:@{ @"name": pad_name }];
    }
}

void onVolumeChanged(RCTGstPlayerView *self, RctGstAudioLevel* audioLevel, gint nb_channels) {
    if (self && self.delegate)
    {
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
        
        [self.delegate volumeChanged:self withData:js_dictionary];
    }
}

void onStateChanged(RCTGstPlayerView *self, GstState old_state, GstState new_state) {
    if (self && self.delegate)
    {
        NSNumber* oldState = [NSNumber numberWithInt:old_state];
        NSNumber* newState = [NSNumber numberWithInt:new_state];
        
        [self.delegate stateChanged:self withData:@{ @"old_state": oldState, @"new_state": newState }];
    }
}

void onUriChanged(RCTGstPlayerView *self, gchar* newUri) {
    if (self && self.delegate)
    {
        NSString *uri = [NSString stringWithUTF8String:newUri];
        NSLog(@"RCTGstPlayer : URI : %@ - LENGTH : %d", uri, uri.length);
        
        [self.delegate uriChanged:self withData:@{ @"new_uri": uri }];
    }
}

void onPlayingProgress(RCTGstPlayerView *self, gint64 progress, gint64 duration) {
    if (self && self.delegate)
    {
        [self.delegate playingProgress:self withData:@{
                                                       @"progress": [NSNumber numberWithInteger:progress],
                                                       @"duration": [NSNumber numberWithInteger:duration]
                                                       }];
    }
}

void onBufferingProgress(RCTGstPlayerView *self, gint progress) {
    if (self && self.delegate)
    {
        [self.delegate bufferingProgress:self withData:@{
                                                     @"progress": [NSNumber numberWithInteger:progress]
                                                     }];
    }
}

void onEOS(RCTGstPlayerView *self)
{
    if (self && self.delegate)
    {
        [self.delegate eos:self];
    }
}

void onElementError(RCTGstPlayerView *self, gchar *source, gchar *message, gchar *debug_info) {
    if (self && self.delegate)
    {
        [self.delegate elementError:self withData:@{
                                                @"source": [NSString stringWithUTF8String:source],
                                                @"message": [NSString stringWithUTF8String:message],
                                                @"debug_info": [NSString stringWithUTF8String:debug_info]
                                                }];
    }
}

void onElementLog(RCTGstPlayerView *self, gchar *newMessage) {
    if (self && self.delegate)
    {
        NSString *message = [NSString stringWithUTF8String:newMessage];
        g_print("%s", newMessage);
        
        [self.delegate elementLog:self withData:@{ @"message": message }];
    }
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Preparing configuration
        [self getUserData]->configuration->onPlayerInit = onPlayerInit;
        [self getUserData]->configuration->onPadAdded = onPadAdded;
        [self getUserData]->configuration->onEOS = onEOS;
        [self getUserData]->configuration->onUriChanged = onUriChanged;
        [self getUserData]->configuration->onPlayingProgress = onPlayingProgress;
        [self getUserData]->configuration->onBufferingProgress = onBufferingProgress;
        [self getUserData]->configuration->onElementError = onElementError;
        [self getUserData]->configuration->onElementLog = onElementLog;
        [self getUserData]->configuration->onStateChanged = onStateChanged;
        [self getUserData]->configuration->onVolumeChanged = onVolumeChanged;
    }
    return self;
}

-(void)layoutSubviews
{
    if (![self isReady]) {
        // Run it
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            rct_gst_run_loop([self getUserData]);
        });
    }
    
    [super layoutSubviews];
}

-(void)removeFromSuperview {
    gst_element_set_state([self getUserData]->playbin, GST_STATE_NULL);
    rct_gst_terminate([self getUserData]);
    [super removeFromSuperview];

}

// Getters
- (guintptr)getHandle
{
    return (guintptr)(id)self;
}

- (RctGstUserData *)getUserData
{
    if (!self->userData) {
        self->userData = rct_gst_init_user_data();
        self->userData->configuration->owner = (__bridge void *)(self);
    }
    return self->userData;
}

- (gboolean)isReady
{
    return [self getUserData]->is_ready;
}

// Methods
- (void)setPipelineState:(int)pipelineState {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        self->pipelineState = pipelineState;
        rct_gst_set_playbin_state([self getUserData], self->pipelineState);
    });
}

- (void)seek:(gint64)position {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        rct_gst_seek([self getUserData], position);
    });
}

@end

