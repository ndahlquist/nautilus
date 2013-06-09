//
//  nativeGraphicsViewController.m
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 4/30/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#import "nativeGraphicsViewController.h"
#import "renderView.h"
#import "common.h"
#import <AVFoundation/AVFoundation.h>

@interface nativeGraphicsViewController ()
@property (nonatomic, retain) IBOutlet renderView *glview;
@property (nonatomic, retain) AVAudioPlayer *effectPlayer;
@end

@implementation nativeGraphicsViewController
@synthesize glview = _glview;
@synthesize effectPlayer;

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self setResources];
    
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    self.glview = [[renderView alloc] initWithFrame:screenBounds];
    [self.view addSubview:self.glview];
	// Do any additional setup after loading the view, typically from a nib.
    
    NSURL* url = [[NSBundle mainBundle] URLForResource:@"bomb_04" withExtension:@"wav"];
    NSAssert(url, @"URL is valid.");
    NSError* error = nil;
    self.effectPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
    if(!self.effectPlayer)
    {
        NSLog(@"Error creating player: %@", error);
    }
}

- (void)setResources
{
    SetResourceCallback(resourceCB);
}

NSString *parseResource(NSString *fileName, NSString *fileType)
{
    NSError *error;
    NSString *fileContents = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:fileName ofType:fileType] usedEncoding:nil error: &error];
    NSLog(@"error:%@", error);
    
    return fileContents;
}

void *resourceCB(const char *cfileName, int * width, int * height)
{
    if(width)
        *width = -1;
    if(height)
        *height = -1;
    
    NSString *fileName = [NSString stringWithFormat:@"%s", cfileName];
    NSArray *fileComponents = [fileName componentsSeparatedByString:@"."];
    NSString *fileType = [fileComponents objectAtIndex:1];
    
    if ([fileType isEqualToString:@"obj"] || [fileType isEqualToString:@"glsl"]) {
        return objResourceCB([fileComponents objectAtIndex:0], fileType);
    } else if ([fileType isEqualToString:@"jpg"] || [fileType isEqualToString:@"jpeg"]) {
        if(width && height)
            return imageResourceCB([fileComponents objectAtIndex:0], fileType, *width, *height);
        LOGE("You should probably have passed width and height here.");
        int tempw, temph;
        return imageResourceCB([fileComponents objectAtIndex:0], fileType, tempw, temph);
    }
    return NULL;
}

void *objResourceCB(NSString *fileName, NSString *fileType)
{
    NSString *fileContents = parseResource(fileName, fileType);
    return strdup([fileContents UTF8String]);
}

void *imageResourceCB(NSString *fileName, NSString *fileType, int & width, int & height)
{
    // Get Core Graphics image referece
    NSString *path = [[NSBundle mainBundle] pathForResource:fileName ofType:fileType];
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    UIImage *image = [[UIImage alloc] initWithData:texData];
    CGImageRef imageRef = image.CGImage;
    
    if (!imageRef) {
        NSLog(@"Failed to load image %@", fileName);
        exit(1);
    }
    
    // Create bitmap context
    width = CGImageGetWidth(imageRef);
    height = CGImageGetHeight(imageRef);
    GLubyte * imageData = (GLubyte *) calloc(width*height*4, sizeof(GLubyte));
    CGContextRef imageContext = CGBitmapContextCreate(imageData, width, height, 8, width*4,
                                                       CGImageGetColorSpace(imageRef), kCGImageAlphaPremultipliedLast);
    // Flip the image
    //CGContextTranslateCTM(imageContext, 0, height);
    //CGContextScaleCTM(imageContext, 1.0, -1.0);
 
    // Draw image into context
    CGContextDrawImage(imageContext, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(imageContext);

    return imageData;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch * touch = [touches anyObject];
    NSUInteger tapCount = [touch tapCount];
    CGPoint location = [touch locationInView:self.view];

    NSArray *locArray = [NSArray arrayWithObjects:[NSNumber numberWithFloat:location.x], [NSNumber numberWithFloat:location.y], nil];
    PointerDown(location.x/self.view.bounds.size.width, location.y/self.view.bounds.size.height);
    switch (tapCount) {
        /*case 1:
            [self performSelector:@selector(singleTapMethod:) withObject:locArray afterDelay:.4];
            break;*/
        case 2:
            //[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(singleTapMethod) object:locArray];
            [self performSelector:@selector(doubleTapMethod) withObject:nil afterDelay:.4];
            break;
    }
    
    
}

- (void)singleTapMethod:(NSArray *)objects
{
    CGPoint location = CGPointMake([[objects objectAtIndex:0] floatValue], [[objects objectAtIndex:1] floatValue]);
    PointerDown(location.x/self.view.bounds.size.width, location.y/self.view.bounds.size.height);
}

- (void)doubleTapMethod
{
    [self.effectPlayer play];
}

// Add new touchesMoved method
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch * touch = [touches anyObject];
    CGPoint location = [touch locationInView:self.view];
    
    PointerMove(location.x/self.view.bounds.size.width, location.y/self.view.bounds.size.height);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch * touch = [touches anyObject];
    CGPoint location = [touch locationInView:self.view];
    
    PointerUp(location.x/self.view.bounds.size.width, location.y/self.view.bounds.size.height);
    
}

- (IBAction)handlePanGesture:(UIPanGestureRecognizer *)sender
{
    
}

@end
