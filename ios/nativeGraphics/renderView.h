//
//  renderView.h
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 4/30/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import "QuartzCore/QuartzCore.h"

@interface renderView : UIView {
    CAEAGLLayer* _eaglLayer;
    EAGLContext* _context;
    GLuint _framebuffer;
    GLuint _colorRenderBuffer;
    GLuint _depthRenderBuffer;
}

@end
