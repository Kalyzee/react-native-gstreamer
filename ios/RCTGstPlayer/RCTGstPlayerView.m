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
void onPlayerInit(RCTGstPlayerView *self)
{
    // Apply what has been stored in instance
    rct_gst_set_uri([self getUserData], [self getUserData]->configuration->uri);
    rct_gst_set_ui_refresh_rate([self getUserData], [self getUserData]->configuration->uiRefreshRate);
    rct_gst_set_volume([self getUserData], [self getUserData]->configuration->volume);
    
    if (self->pipelineState != NULL)
        [self setPipelineState:self->pipelineState];
    
    rct_gst_set_drawable_surface([self getUserData], [self getHandle]);
    
    self.onPlayerInit(@{});
}

void onPadAdded(RCTGstPlayerView *self, gchar *name)
{
    NSString *pad_name = [NSString stringWithUTF8String:name];
    self.onPadAdded(@{ @"name": pad_name });
}

void onEOS(RCTGstPlayerView *self)
{
    self.onEOS(@{});
}

void onUriChanged(RCTGstPlayerView *self, gchar* newUri) {
    NSString *uri = [NSString stringWithUTF8String:newUri];
    NSLog(@"RCTGstPlayer : URI : %@ - LENGTH : %d", uri, uri.length);
    self.onUriChanged(@{ @"new_uri": uri });
}

void onPlayingProgress(RCTGstPlayerView *self, gint64 progress, gint64 duration) {
    self.onPlayingProgress(@{
                               @"progress": [NSNumber numberWithInteger:progress],
                               @"duration": [NSNumber numberWithInteger:duration]
                               });
}

void onBufferingProgress(RCTGstPlayerView *self, gint progress) {
    self.onBufferingProgress(@{
                                 @"progress": [NSNumber numberWithInteger:progress]
                                 });
}

void onElementError(RCTGstPlayerView *self, gchar *source, gchar *message, gchar *debug_info) {
    self.onElementError(@{
                            @"source": [NSString stringWithUTF8String:source],
                            @"message": [NSString stringWithUTF8String:message],
                            @"debug_info": [NSString stringWithUTF8String:debug_info]
                            });
}

void onStateChanged(RCTGstPlayerView *self, GstState old_state, GstState new_state) {
    
    NSNumber* oldState = [NSNumber numberWithInt:old_state];
    NSNumber* newState = [NSNumber numberWithInt:new_state];

    self.onStateChanged(@{ @"old_state": oldState, @"new_state": newState });
}

void onVolumeChanged(RCTGstPlayerView *self, RctGstAudioLevel* audioLevel, gint nb_channels) {
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
    self.onVolumeChanged(js_dictionary);
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

