//  RenderObject.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderObject__
#define __nativeGraphics__RenderObject__

#include <iostream>
#include <string>

#include "graphics_header.h"

#include <vector>

using namespace std;

class RenderObject {
public:
    // Basic constructor (no geometry)
    RenderObject(const char *vertexShaderFile, const char *fragmentShaderFile, bool writegeometry = true);

    // Constructor with geometry
    RenderObject(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile, bool writegeometry = true);

    // Add a texture or normal map
    void AddTexture(const char *textureFilename, bool normalmap = false);

    // Render color and geometry to g buffer
    void Render(int instance = 0, GLfloat *buffer = 0, int num = -1);

    GLuint colorShader;
    GLuint geometryShader; // NULL, except when a custom v shader is specified.

protected:
    void BasicInit(const char *vertexShaderFilename, const char *fragmentShaderFilename, bool writegeometry);
    void SetShader(const GLuint shaderProgram);
    virtual void RenderPass(int instance, GLfloat *buffer, int num);

    GLuint gvPositionHandle;
    GLuint gmvMatrixHandle;
    GLuint gmvpMatrixHandle;
    GLuint textureUniform;
    GLuint gvTexCoords;
    GLuint gvNormals;
    GLuint gVertexBuffer;
    GLuint normalMapUniform;
    GLuint timeUniform;

    GLuint texture;
    GLuint normalTexture;
    int numVertices;
};


#endif // __nativeGraphics__RenderObject__
