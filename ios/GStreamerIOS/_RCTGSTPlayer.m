//
//  RCTGSTPlayer.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 21/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import <React/RCTConvert.h>
#import <React/RCTEventDispatcher.h>
#import "_RCTGSTPlayer.h"
#import <React/RCTLog.h>

@interface RCTGSTPlayer() {
  GStreamerBackend *gst_backend;
  int media_width;
  int media_height;
}
@end

@implementation RCTGSTPlayer

- (void)viewDidLoad
{
  NSLog(@"OK");
}

- (instancetype)initWithEventDispatcher:(RCTEventDispatcher *)eventDispatcher
{
  if ((self = [super init]))
  {
    _eventDispatcher = eventDispatcher;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground:)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    
    gst_backend = [[GStreamerBackend alloc] init:self videoView:video_view];
    
    // My UI elements
    // Hello world label
    label = [[UILabel alloc] initWithFrame:CGRectMake(0, 30, 250, 30)];
    label.text = [NSString stringWithFormat:@"Welcome to %@!", [gst_backend getGStreamerVersion]];
    [self addSubview:label];

    // Buttons
    play_button = [[UIButton alloc] initWithFrame:CGRectMake(0, 80, 100, 30)];
    play_button.enabled = FALSE;
    [play_button setTitle:@"Play" forState:UIControlStateNormal];
    [play_button setBackgroundColor:[UIColor blackColor]];
    [play_button addTarget:self action:@selector(playButtonAction:) forControlEvents:UIControlEventTouchDown];
    [self addSubview:play_button];

    pause_button = [[UIButton alloc] initWithFrame:CGRectMake(120, 80, 100, 30)];
    pause_button.enabled = FALSE;
    [pause_button setTitle:@"Pause" forState:UIControlStateNormal];
    [pause_button setBackgroundColor:[UIColor blackColor]];
    [pause_button addTarget:self action:@selector(pauseButtonAction:) forControlEvents:UIControlEventTouchDown];
    [self addSubview:pause_button];
    
    
    // Message label
    message_label = [[UILabel alloc] initWithFrame:CGRectMake(0, 120, 250, 30)];
    message_label.text = @"GStreamer Messages";
    [self addSubview:message_label];
    
    // Video container
    video_container_view = [[UIView alloc] initWithFrame:CGRectMake(0, 150, 300, 400)];
    [video_container_view setBackgroundColor:[UIColor blackColor]];
    [self addSubview:video_container_view];
    
    // Video view
    video_view = [[EaglUIView alloc] initWithFrame:CGRectMake(0, 0, 320, 240)];
    [video_view setBackgroundColor:[UIColor blueColor]];
    [video_container_view addSubview:video_view];

    
    // View constraints
    media_width = 320;
    media_height = 240;
    
    CGFloat view_width = video_container_view.bounds.size.width;
    CGFloat view_height = video_container_view.bounds.size.height;
    
    CGFloat correct_height = view_width * media_height / media_width;
    CGFloat correct_width = view_height * media_width / media_height;
    
    if (correct_height < view_height) {
      video_height_constraint.constant = correct_height;
      video_width_constraint.constant = view_width;
    } else {
      video_width_constraint.constant = correct_width;
      video_height_constraint.constant = view_height;
    }
    
  }
  
  return self;
}

/* Called when the Play button is pressed */
-(void) playButtonAction:(UIButton *)button
{
  [gst_backend play];
}

/* Called when the Pause button is pressed */
-(void) pauseButtonAction:(UIButton *)button
{
  [gst_backend pause];
}


/*
 * Methods from GstreamerBackendDelegate
 */

-(void) gstreamerInitialized
{
  dispatch_async(dispatch_get_main_queue(), ^{
    play_button.enabled = TRUE;
    pause_button.enabled = TRUE;
    message_label.text = @"Ready";
  });
}

-(void) gstreamerSetUIMessage:(NSString *)message
{
  dispatch_async(dispatch_get_main_queue(), ^{
    message_label.text = message;
  });
}

@end
