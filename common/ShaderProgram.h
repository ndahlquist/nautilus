// glsl_helper.h
// nativeGraphics
// Compiles and links GLSL shader programs

#ifndef __nativeGraphics__ShaderProgram__
#define __nativeGraphics__ShaderProgram__

class ShaderProgram {
public:
    ShaderProgram(const char *vertexShaderFile = NULL, const char *fragmentShaderFile = NULL);
    
    GLuint programHandle;
}

#endif // __nativeGraphics__ShaderProgram__
