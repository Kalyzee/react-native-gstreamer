//
//  EaglUiView.m
//
//  Created by Alann on 13/12/2017.
//  Copyright © 2017 Kalyzee. All rights reserved.
//

#import "RCTGstPlayerView.h"

@implementation RCTGstPlayerView

static gboolean shareInstance;
static RCTGstPlayerView *latestView;

RCTGstPlayerView *c_view;

// GL surface
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

+ (RCTGstPlayerView *)getView
{
    if (shareInstance) {
        if (!latestView)
            latestView = [[RCTGstPlayerView alloc] init];
    } else {
        latestView = [[RCTGstPlayerView alloc] init];
    }
    
    NSLog(@"Returning View : %@", latestView);
    return latestView;
}

// Constructor
- (instancetype)init
{
    self->isReady = FALSE;
    self->pipelineState = NULL;
    
    c_view = (id)self;
    return [super init];
}

// Callables
void onInit()
{
    c_view.onPlayerInit(@{});
}

void onEOS()
{
    c_view.onEOS(@{});
}

void onUriChanged(gchar* newUri) {
    NSString *uri = [NSString stringWithUTF8String:newUri];
    NSLog(@"RCTGstPlayer : URI : %@ - LENGTH : %d", uri, uri.length);
    if (c_view.onUriChanged)
        c_view.onUriChanged(@{ @"new_uri": uri });
}

void onElementError(gchar *source, gchar *message, gchar *debug_info) {
    c_view.onElementError(@{
                            @"source": [NSString stringWithUTF8String:source],
                            @"message": [NSString stringWithUTF8String:message],
                            @"debug_info": [NSString stringWithUTF8String:debug_info]
                            });
}

void onStateChanged(GstState old_state, GstState new_state) {
    
    NSNumber* oldState = [NSNumber numberWithInt:old_state];
    NSNumber* newState = [NSNumber numberWithInt:new_state];
    
    c_view.onStateChanged(@{ @"old_state": oldState, @"new_state": newState });
}

void onVolumeChanged(RctGstAudioLevel* audioLevel) {
    c_view.onVolumeChanged(@{
                             @"decay": @(audioLevel->decay),
                             @"rms": @(audioLevel->rms),
                             @"peak": @(audioLevel->peak),
                             });
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        self->isReady = FALSE;
        
        // Preparing configuration
        [self getUserData]->configuration->onInit = onInit;
        [self getUserData]->configuration->onEOS = onEOS;
        [self getUserData]->configuration->onUriChanged = onUriChanged;
        [self getUserData]->configuration->onElementError = onElementError;
        [self getUserData]->configuration->onStateChanged = onStateChanged;
        [self getUserData]->configuration->onVolumeChanged = onVolumeChanged;
    }
    return self;
}

-(void)willMoveToSuperview:(UIView *)newSuperview {
    [super willMoveToSuperview:newSuperview];
    
    if (!self->isReady) {
        NSLog(@"willMoveToSuperview");
        
        self->isReady = TRUE;
        [self getUserData]->configuration->drawableSurface = [self getHandle];
        
        // Preparing pipeline
        rct_gst_init([self getUserData]);
        
        // Apply once ready what has been stored in instance
        rct_gst_apply_uri([self getUserData]);
        rct_gst_set_audio_level_refresh_rate([self getUserData], [self getUserData]->configuration->audioLevelRefreshRate);
        rct_gst_set_debugging([self getUserData], [self getUserData]->configuration->isDebugging);
        rct_gst_set_volume([self getUserData], [self getUserData]->configuration->volume);
        
        if (self->pipelineState != NULL)
            [self setPipelineState:self->pipelineState];
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            rct_gst_run_loop([self getUserData]);
        });
    }
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
    return self->isReady;
}

//Setters
- (void)setPipelineState:(int)pipelineState {
    self->pipelineState = pipelineState;
    rct_gst_set_pipeline_state([self getUserData], self->pipelineState);
}

- (void)setShareInstance:(BOOL)_shareInstance {
    shareInstance = _shareInstance;
}

@end

