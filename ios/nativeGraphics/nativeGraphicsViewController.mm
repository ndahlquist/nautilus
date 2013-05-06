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

NSString* parseResource(NSString *fileName, NSString *fileType)
{
    NSError *error;
    NSString *fileContents = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:fileName ofType:fileType] usedEncoding:nil error: &error];
    NSLog(@"error:%@", error);
    
    return fileContents;
}

char *resourceCB(const char *cfileName)
{
    NSString *fileName = [NSString stringWithFormat:@"%s", cfileName];
    NSArray *fileComponents = [fileName componentsSeparatedByString:@"."];
    NSString *fileContents = parseResource([fileComponents objectAtIndex:0], [fileComponents objectAtIndex:1]);
    
    return strdup([fileContents UTF8String]);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
