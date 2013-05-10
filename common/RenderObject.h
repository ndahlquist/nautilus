//
//  RenderObject.h
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 5/9/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#ifndef __nativeGraphics__RenderObject__
#define __nativeGraphics__RenderObject__

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


#include "transform.h"

using namespace std;

class RenderObject {
public:
    RenderObject(char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void RenderFrame();
    
    
private:
    GLuint gProgram;
    GLuint gvPositionHandle;
    GLuint gmvMatrixHandle;
    GLuint gmvpMatrixHandle;
    GLuint textureUniform;
    GLuint gvTexCoords;
    GLuint gvNormals;
    GLuint gVertexBuffer;

    
    int numVertices = 0;
    

    
};


#endif /* defined(__nativeGraphics__RenderObject__) */
