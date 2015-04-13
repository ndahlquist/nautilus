//  RenderPipeline.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderPipeline__
#define __nativeGraphics__RenderPipeline__

#include "graphics_header.h"

#include "Eigen/Core"

class RenderPipeline {
public:

    RenderPipeline();
    void ClearBuffers();
    uint8_t getDepth(float x, float y);
    Eigen::Vector3f getNormal(float x, float y, Eigen::Matrix4f mvp);

    GLuint frameBuffer;

    GLuint gBuffer; // R, G, B, Depth_MVP
    GLuint depthBuffer;

};

#endif // __nativeGraphics__RenderPipeline__
