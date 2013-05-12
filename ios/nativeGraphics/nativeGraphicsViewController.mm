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

@interface nativeGraphicsViewController ()
@property (nonatomic, retain) IBOutlet renderView *glview;
@end

@implementation nativeGraphicsViewController
@synthesize glview = _glview;

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self setResources];
    
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    self.glview = [[renderView alloc] initWithFrame:screenBounds];
    [self.view addSubview:self.glview];
	// Do any additional setup after loading the view, typically from a nib.
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

void *resourceCB(const char *cfileName)
{
    NSString *fileName = [NSString stringWithFormat:@"%s", cfileName];
    NSArray *fileComponents = [fileName componentsSeparatedByString:@"."];
    NSString *fileType = [fileComponents objectAtIndex:1];
    
    if ([fileType isEqualToString:@"obj"] || [fileType isEqualToString:@"glsl"]) {
        return objResourceCB([fileComponents objectAtIndex:0], fileType);
    } else if ([fileType isEqualToString:@"jpg"]) {
        return imageResourceCB([fileComponents objectAtIndex:0], fileType);
    }
    return NULL;
}

void *objResourceCB(NSString *fileName, NSString *fileType)
{
    NSString *fileContents = parseResource(fileName, fileType);
    return strdup([fileContents UTF8String]);
}

void *imageResourceCB(NSString *fileName, NSString *fileType)
{
    // 1
    NSString *path = [[NSBundle mainBundle] pathForResource:fileName ofType:fileType];
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    UIImage *image = [[UIImage alloc] initWithData:texData];
    CGImageRef spriteImage = image.CGImage;
    if (!spriteImage) {
        NSLog(@"Failed to load image %@", fileName);
        exit(1);
    }
    
    // 2
    size_t width = CGImageGetWidth(spriteImage);
    size_t height = CGImageGetHeight(spriteImage);
    
    GLubyte * spriteData = (GLubyte *) calloc(width*height*4, sizeof(GLubyte));
    
    CGContextRef spriteContext = CGBitmapContextCreate(spriteData, width, height, 8, width*4,
                                                       CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
    
    // 3
    CGContextDrawImage(spriteContext, CGRectMake(0, 0, width, height), spriteImage);
    
    CGContextRelease(spriteContext);
    return spriteData;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch * touch = [touches anyObject];
    CGPoint location = [touch locationInView:self.view];
    
    PointerDown(location.x/self.view.bounds.size.width, location.y/self.view.bounds.size.height);
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
