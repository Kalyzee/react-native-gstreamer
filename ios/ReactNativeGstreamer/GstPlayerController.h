//
//  RCTGSTPlayerController.h
//  GStreamerIOS
//
//  Created by Alann Sapone on 26/07/2017.
//  Copyright © 2017 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "GStreamerBackendDelegate.h"
#import "GStreamerBackend.h"


@interface GstPlayerController : UIViewController <GStreamerBackendDelegate> {
    IBOutlet UILabel *version_label;
    IBOutlet UILabel *message_label;
    IBOutlet UIView *video_view;
    IBOutlet UIView *video_container_view;
    IBOutlet NSLayoutConstraint *video_width_constraint;
    IBOutlet NSLayoutConstraint *video_height_constraint;
    IBOutlet UIStackView *footer_stack_view;
    int media_width;
    int media_height;
}

@property (retain, nonatomic) NSString *uri;
@property BOOL play;
@property (retain, nonatomic) GStreamerBackend *gst_backend;

-(IBAction) play:(id)sender;
-(IBAction) pause:(id)sender;

/* From GStreamerBackendDelegate */
-(void) gstreamerInitialized;
-(void) gstreamerSetUIMessage:(NSString *)message;
-(void) setUri:(NSString *)_uri;

@end
