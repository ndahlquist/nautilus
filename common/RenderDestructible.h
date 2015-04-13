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

#include "graphics_header.h"

#include "RenderObject.h"

using namespace std;
#include <vector>
/*
struct DestructibleBond;
struct DestructibleNode;
struct DestructibleCell;
struct DestructibleFace;
*/
class RenderDestructible : public RenderObject {
public:
    RenderDestructible(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Render();
    void RenderPass(int instance, GLfloat *buffer, int num);
    GLfloat * getGeometry(int & num_vertices);

    bool explode;
    int ***voxelGrid3D;
};


#endif /* defined(__nativeGraphics__RenderTest__) */
