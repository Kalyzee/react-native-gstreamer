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

-(id) init:(id) uiDelegate videoView:(UIView*) video_view;

-(void) deinit;

-(NSString*) getGStreamerVersion;

-(void) play;

-(void) pause;

-(void) setUriSource:(NSString*)_uriSource;

@end

#endif /* GStreamerBackend_h */
