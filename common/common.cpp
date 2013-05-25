//  common.cpp
//  nativeGraphics

#include "common.h"

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

#include "Eigen/Core"
#include "Eigen/Eigenvalues"

#include "transform.h"
#include "RenderObject.h"
#include "RenderLight.h"
#include "glsl_helper.h"
#include "log.h"

using namespace std;

int width = 0;
int height = 0;

GLuint frameBuffer;
GLuint depthBuffer;

GLuint colorTexture; // R, G, B, UNUSED (specular)
GLuint geometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP

GLuint albedoShader;
GLuint brownShader;
GLuint normalsShader;

RenderObject *cave;
RenderObject *character;
RenderLight *light;

float cameraPos[4] = {0,0,0.9,1};
float pan[3] = {0,0,0}, up[3] = {0,1,0};
float rot[2] = {0,0};

unsigned int frameNum = 0;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

GLuint defaultFrameBuffer = 0;

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
void Setup(int w, int h) {
    
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }
    
    cave = new RenderObject("cave0.obj", "standard_v.glsl", "solid_color_f.glsl");
    character = new RenderObject("raptor.obj", "standard_v.glsl", "albedo_f.glsl");
    character->AddTexture("raptor_albedo.jpg");
    light = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_f.glsl");
    
    albedoShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("albedo_f.glsl"));
    brownShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("solid_color_f.glsl"));
    normalsShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("normals_f.glsl"));
    
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    
    // Allocate frame buffer
	glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    // Allocate depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    // Allocate albedo texture to render to.
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("colorTexture");
    light->colorTexture = colorTexture;
    
    // Allocate normal texture to render to.
    glGenTextures(1, &geometryTexture);
    glBindTexture(GL_TEXTURE_2D, geometryTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("geometryTexture");
    light->geometryTexture = geometryTexture;
    
    glBindTexture(GL_TEXTURE_2D, 0);       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderFrame() {

    glViewport(0, 0, width, height);

    pLoadIdentity();
    perspective(20, (float) width / (float) height, 80, 180);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);

    //////////////////////////////////
    // Render to frame buffer
    
    // Render colors (R, G, B, UNUSED / SPECULAR)
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glClearColor(0., 0., 0., 0.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    rotate(rot[1],rot[0],0);
    translatef(0.0f, -40.0f, 0.0f);
    cave->SetShader(brownShader);
    cave->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    rotate(rot[1],rot[0],0);
    translatef(68.0f, -40.0f, -20.0f);
    character->SetShader(albedoShader);    
    character->RenderFrame();
    mvPopMatrix();
    
    // Render geometry (NX_MV, NY_MV, NZ_MV, Depth_MVP)
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    rotate(rot[1],rot[0],0);
    translatef(0.0f, -40.0f, 0.0f);
    cave->SetShader(normalsShader);
    cave->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    rotate(rot[1],rot[0],0);
    translatef(68.0f, -40.0f, -20.0f);
    character->SetShader(normalsShader);    
    character->RenderFrame();
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Render from frame buffer

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    
    glUseProgram(light->gProgram);
    
    GLuint u_FragWidth = glGetUniformLocation(light->gProgram, "u_FragWidth");
    glUniform1i(u_FragWidth, width);
    
    GLuint u_FragHeight = glGetUniformLocation(light->gProgram, "u_FragHeight");
    glUniform1i(u_FragHeight, height);
    
    float lightScale = 15.0f;
    
    for(int i = 0; i < 3; i++)
        light->brightness[i] = 40;
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(3.0 * cos(frameNum / 50.0f) / lightScale, -10.0f / lightScale, 30.0 * sin(frameNum / 100.0f) / lightScale);
    light->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(16.0 * sin(frameNum / 20.0f) / lightScale, -10.0f / lightScale, 16.0 * cos(frameNum / 20.0f) / lightScale);
    light->RenderFrame();
    mvPopMatrix();
    
    lightScale = 20.0f;
    
    for(int i = 0; i < 3; i++)
        light->brightness[i] = 120;
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(-6.0f / lightScale, 10.0f / lightScale, 0.0f);
    light->RenderFrame();
    mvPopMatrix();
    
    frameNum++;
}

// User interface functions:

float lastPointer[2] = {0,0};

void PointerDown(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

void PointerMove(float x, float y, int pointerIndex) {
     float deltaX = x - lastPointer[0];
	 float deltaY = y - lastPointer[1];
	 
	 rot[0] +=  8.0 * deltaX;
	 rot[1] += -1.0 * deltaY;
	 
	 lastPointer[0] = x;
     lastPointer[1] = y;
}

void PointerUp(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

