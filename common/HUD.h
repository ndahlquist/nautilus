//  HUD.h
//  nativeGraphics

#ifndef __nativeGraphics__HUD__
#define __nativeGraphics__HUD__

#include <iostream>
#include <string>

#include "graphics_header.h"

#include "RenderObject.h"
#include "Eigen/Core"

using namespace std;

class HUD : public RenderObject {
public:
    HUD();
    void Render(float health);
    void ShowRadar(Eigen::Vector3f delta_pos, int color, float size);
    void RenderElement(GLuint textureHandle, float xdisp, float ydisp, float xscale, float yscale);
    GLuint AddTexture(const char *textureFilename);

    GLuint healthbarTex;
    GLuint healthbarBorderTex;
    GLuint radarTex;
    GLuint greenDotTex;
    GLuint redDotTex;
};


#endif // __nativeGraphics__HUD__
