// glsl_helper.cpp
// nativeGraphics
// Compiles and links GLSL shader programs

#include "glsl_helper.h"

#include <cstdlib>
#include <cstdio>

#include "log.h"

static const char defaultVertexShader[] =
"attribute vec4 a_Position;\n"
"void main() {\n"
"  gl_Position = a_Position;\n"
"}\n";

static const char defaultFragmentShader[] =
#ifdef ANDROID_NDK
"precision mediump float;\n"
#endif
"void main() {\n"
"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
"}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if(buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
                    LOGI("%s", pSource);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createShaderProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader;
    if(pVertexSource)
        vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    else
        vertexShader = loadShader(GL_VERTEX_SHADER, defaultVertexShader);
    
    GLuint pixelShader;
    if(pFragmentSource)
        pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    else
        pixelShader = loadShader(GL_FRAGMENT_SHADER, defaultFragmentShader);
    
    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}
