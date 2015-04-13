//  RenderLight.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderLight__
#define __nativeGraphics__RenderLight__

#include <iostream>
#include <string>

#include "graphics_header.h"

#include "RenderObject.h"

using namespace std;

class RenderLight : public RenderObject {
public:
    RenderLight(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Render();

    float color[3]; // R, G, B (0.0, 1.0)
    float brightness; // (0, inf)
};


#endif // __nativeGraphics__RenderLight__
