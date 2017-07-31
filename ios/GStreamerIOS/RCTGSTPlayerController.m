//
//  RCTGSTPlayerController.m
//  GStreamerIOS
//
//  Created by Alann Sapone on 26/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import "RCTGSTPlayerController.h"
#import <UIKit/UIKit.h>

@implementation RCTGSTPlayerController

@synthesize uri;
@synthesize gst_backend;

/*
 * Methods from RCTGSTPlayerController
 */

- (void)viewDidLoad
{
  [super viewDidLoad];
  gst_backend = [[GStreamerBackend alloc] init:self videoView:video_view];
}

-(void)setDefaultUri:(NSString*)_uriSource
{
  uri = _uriSource;
}

- (void)viewDidDisappear:(BOOL)animated
{
  if (gst_backend)
  {
    [gst_backend deinit];
  }
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

/* Called when the Play button is pressed */
-(IBAction) play:(id)sender
{
  [gst_backend play];
}

/* Called when the Pause button is pressed */
-(IBAction) pause:(id)sender
{
  [gst_backend pause];
}

/*
 * Methods from GstreamerBackendDelegate
 */

-(void) gstreamerInitialized
{
  dispatch_async(dispatch_get_main_queue(), ^{
    message_label.text = @"Ready";
    version_label.text = [NSString stringWithFormat:@"GStreamer Version : %@", [gst_backend getGStreamerVersion]];
    
    [gst_backend setUriSource:uri];
    NSLog(@"%@", uri);
    [self play:self];
  });
}

-(void) gstreamerSetUIMessage:(NSString *)message
{
  dispatch_async(dispatch_get_main_queue(), ^{
    message_label.text = message;
    NSLog(@"%@", message);
  });
}

@end
