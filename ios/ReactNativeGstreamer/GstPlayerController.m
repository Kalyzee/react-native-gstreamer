//
//  RCTGSTPlayerController.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 26/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import "GstPlayerController.h"
#import <UIKit/UIKit.h>
#include <gst/gst.h>
#include "EaglUIView.h"

@implementation GstPlayerController

@synthesize uri;
@synthesize play;
@synthesize gst_backend;

/*
 * Methods from RCTGSTPlayerController
 */

- (void)viewDidLoad
{
    [super viewDidLoad];
    gst_backend = [[GStreamerBackend alloc] init:self videoView:video_view];
    
}

/* Called when the size of the main view has changed, so we can
 * resize the sub-views in ways not allowed by storyboarding. */
- (void)viewDidLayoutSubviews
{
}

-(void)setUri:(NSString *)_uri
{
    [gst_backend setUri:_uri];
}

-(void)setPlay:(BOOL)_play
{
    NSLog(@"State of play : %@", _play == YES ? @"YES" : @"NO");
    [gst_backend setPlay:_play];
}

- (void)viewDidDisappear:(BOOL)animated
{
    if (gst_backend)
    {
        NSLog(@"DEINITING GSTREAMER");
        [gst_backend deinit];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
 * Methods from GstreamerBackendDelegate
 */

-(void) gstreamerInitialized
{
    dispatch_async(dispatch_get_main_queue(), ^{
        message_label.text = @"Ready";
    });
}

-(void) gstreamerSetUIMessage:(NSString *)message
{
    dispatch_async(dispatch_get_main_queue(), ^{
        message_label.text = message;
    });
}

-(void) audioLevelChanged:(double)audioLevel
{
    dispatch_async(dispatch_get_main_queue(), ^{
        
        EaglUIView* view = (EaglUIView*)self.view;
        if (!view.onAudioLevelChange)
            return;
        
        view.onAudioLevelChange(@{ @"level": @(audioLevel) });
    });
}

@end
