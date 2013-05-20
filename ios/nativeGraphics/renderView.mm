//
//  renderView.m
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 4/30/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#import "renderView.h"
#import "common.h"
#import <AVFoundation/AVFoundation.h>

@interface renderView()
@property (nonatomic, retain) AVAudioPlayer *player;
@end

@implementation renderView
@synthesize player = _player;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        _eaglLayer = (CAEAGLLayer *)[self layer];
        
        [self setupLayer];
        [self setupContext];
        Setup(self.bounds.size.width, self.bounds.size.height);
        [self setupDepthBuffer];
        [self setupRenderBuffer];
        [self setupFrameBuffer];
        [self setupDisplayLink];
        
       

    }
    return self;
}

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (void)setupLayer {
    _eaglLayer = (CAEAGLLayer*) self.layer;
    _eaglLayer.opaque = YES;
}

- (void)setupContext {
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    _context = [[EAGLContext alloc] initWithAPI:api];
    if (!_context) {
        NSLog(@"Failed to initialize OpenGLES 2.0 context");
        exit(1);
    }
    
    if (![EAGLContext setCurrentContext:_context]) {
        NSLog(@"Failed to set current OpenGL context");
        exit(1);
    }
}

- (void)setupDepthBuffer {
    glGenRenderbuffers(1, &_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, self.frame.size.width, self.frame.size.height);
}

- (void)setupRenderBuffer {
    glGenRenderbuffers(1, &_colorRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];
}

- (void)setupFrameBuffer {
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, _colorRenderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
    setFrameBuffer(_framebuffer);
}

- (void)setupDisplayLink {
    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (NSURL *)getSoundURL
{
    int sound = rand()%4;
    if (sound == 1)
        return [[NSBundle mainBundle] URLForResource:@"water-droplet-1" withExtension:@"wav"];
    else if (sound == 2)
        return [[NSBundle mainBundle] URLForResource:@"water-droplet-2" withExtension:@"wav"];
    else if (sound == 3)
        return [[NSBundle mainBundle] URLForResource:@"water-droplets-1" withExtension:@"wav"];
    else
        return [[NSBundle mainBundle] URLForResource:@"water-droplets-2" withExtension:@"wav"];
}

- (void)render:(CADisplayLink*)displayLink
{
    static int time = 0;
    static int interval = 60;
    
    if (time >= interval) {
        NSURL* url = [self getSoundURL];
        NSAssert(url, @"URL is valid.");
        NSError* error = nil;
        self.player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
        if(!self.player)
        {
            NSLog(@"Error creating player: %@", error);
        }
        [self.player play];
        interval += 60 * (rand()%4 + 1);
    }
    time++;
    
    RenderFrame();
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
/*- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
