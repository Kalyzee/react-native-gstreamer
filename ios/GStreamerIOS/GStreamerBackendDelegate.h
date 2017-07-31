//
//  GStreamerBackendDelegate.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 25/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#ifndef GStreamerBackendDelegate_h
#define GStreamerBackendDelegate_h

#import <Foundation/Foundation.h>

@protocol GStreamerBackendDelegate <NSObject>

@optional
/* Called when the GStreamer backend has finished initializing
 * and is ready to accept orders. */
-(void) gstreamerInitialized;

/* Called when the GStreamer backend wants to output some message
 * to the screen. */
-(void) gstreamerSetUIMessage:(NSString *)message;

/* Called when the media size is first discovered or it changes */
-(void) mediaSizeChanged:(NSInteger)width height:(NSInteger)height;

/* Called when the media position changes. Times in milliseconds */
-(void) setCurrentPosition:(NSInteger)position duration:(NSInteger)duration;

@end



#endif /* GStreamerBackendDelegate_h */
