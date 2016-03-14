//
//  OpenGLView.h
//  me_app
//
//  Created by hahasasa on 3/15/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#ifndef OpenGLView_h
#define OpenGLView_h


#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>


@interface OpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
    NSTimer* timer;
}

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format;

@end


#endif /* OpenGLView_h */
