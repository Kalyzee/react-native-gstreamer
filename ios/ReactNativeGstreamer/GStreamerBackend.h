//
//  GStreamerBackend.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 24/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#ifndef GStreamerBackend_h
#define GStreamerBackend_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "GStreamerBackendDelegate.h"

@interface GStreamerBackend : NSObject

-(NSString*) getGStreamerVersion;

/* Initialization method. Pass the delegate that will take care of the UI.
 * This delegate must implement the GStreamerBackendDelegate protocol.
 * Pass also the UIView object that will hold the video window. */
-(id) init:(id) uiDelegate videoView:(UIView*) video_view;

/* Quit the main loop and free all resources, including the pipeline and
 * the references to the ui delegate and the UIView used for rendering, so
 * these objects can be deallocated. */
-(void) deinit;

/* Set the pipeline to PLAYING */
-(void) play;

/* Set the pipeline to PAUSED */
-(void) pause;

/* Set the URI to be played */
-(void) setUri:(NSString*)uri;

/* Set the position to seek to, in milliseconds */
-(void) setPosition:(NSInteger)milliseconds;

@end

#endif /* GStreamerBackend_h */
