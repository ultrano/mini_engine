//
//  OpenGLView.m
//  me_app
//
//  Created by hahasasa on 3/15/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OpenGLView.h"
#import "MiniEngine.h"

@implementation OpenGLView

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format
{
    
    NSOpenGLPixelFormat* _pixelFormat;
    self = [super initWithFrame:frameRect];
    if (self != nil)
    {
        //_pixelFormat   = [format retain];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_surfaceNeedsUpdate:)
                                                     name:NSViewGlobalFrameDidChangeNotification
                                                   object:self];
    }
    return self;
}

- (void) _surfaceNeedsUpdate:(NSNotification*)notification
{
    [self update];
}

- (void)renewGState
{
    // Called whenever graphics state updated (such as window resize)
    
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];
    
    [super renewGState];
}

-(void)prepareOpenGL
{
    
    NSString *resourceDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"resource"];
    const char* resourceFolder = [resourceDir UTF8String];
    
    MNStart(resourceFolder, "script/test.mn");
    
    timer = [NSTimer timerWithTimeInterval:0
                                    target:self
                                  selector:@selector(timerEvent:)
                                  userInfo:self
                                   repeats:YES];
    
    [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
    
}

- (void)timerEvent:(NSTimer *)t
{
    MNUpdate();
}

- (void) drawRect: (NSRect) theRect
{
    [[self openGLContext] makeCurrentContext];
    //Perform drawing here
    
    printf("22");
    
    [[self openGLContext] flushBuffer];
}

- (void)update
{
    printf("33");
}

@end
