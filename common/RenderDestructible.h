//
//  RenderTest.h
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 6/2/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#ifndef __nativeGraphics__RenderTest__
#define __nativeGraphics__RenderTest__

#include <iostream>
#include <string>

#ifdef ANDROID_NDK
#include "importgl.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#elif __APPLE__
#include <stdlib.h>
#include <OpenGLES/ES2/gl.h>
#else // linux
#include <GL/glew.h>
#include <stdio.h>
#endif

#include "RenderObject.h"

using namespace std;
#include <vector>

struct DestructibleBond;
struct DestructibleNode;
struct DestructibleCell;
struct DestructibleFace;

class RenderDestructible : public RenderObject {
public:
    RenderDestructible(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Render();
    void RenderPass();
    
    int **voxelGrid3D;
};


#endif /* defined(__nativeGraphics__RenderTest__) */
