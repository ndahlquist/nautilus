//  common.cpp
//  nativeGraphics

#include "common.h"

#include "obj_parser.h"

#include <string>

#include "Eigen/Core"
#include "Eigen/Eigenvalues"

#define  LOG_TAG    "libnativegraphics"
#include "log.h"

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

using namespace std;

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

// File locations
string vertexShader;
string fragmentShader;
string normalMap;
string lightProbe;

// Light source attributes
static float specularLight[] = {1.00, 1.00, 1.00, 1.0};
static float ambientLight[]  = {0.10, 0.10, 0.10, 1.0};
static float diffuseLight[]  = {1.00, 1.00, 1.00, 1.0};

// Material color properties
static float materialAmbient[]  = { 0.2, 0.2, 0.6, 1.0 };
static float materialDiffuse[]  = { 0.2, 0.2, 0.6, 1.0 };
static float materialSpecular[] = { 0.8, 0.8, 0.8, 1.0 };
static float shininess          = 8.0;  // # between 1 and 128.

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
         = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] =
"attribute vec4 vPosition;\n"
"void main() {\n"
"  gl_Position = vPosition;\n"
"}\n";

static const char gFragmentShader[] =
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

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if(!vertexShader)
        return 0;
    
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if(!pixelShader)
        return 0;
    
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

GLuint gProgram;
GLuint gvPositionHandle;
GLuint textureUniform;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

GLfloat * raptorVertices = NULL;
int raptorVerticesSize = 0;

//
// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
//
void Setup(int w, int h) {
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(0);
    }

    /////////////////
    std::vector<struct Vertex> vertices;
	std::vector<struct face> faces;
	
	char * objFile = (char *)resourceCallback("raptor.obj");
	parseObjString(objFile, vertices, faces);
	free(objFile);

	computeAdjacencyLists(vertices, faces);

    /*for(int i = 0; i < subdivide; i++) {
	    subdivideMesh(vertices, faces);
	    computeAdjacencyLists(vertices, faces);
	    smoothMesh(vertices, faces);
	}*/

	computeNormals(vertices, faces);

	const int faceSize = 3*3 + 3*3 + 3*2;
	const int size = faces.size() * faceSize;

    raptorVertices = (GLfloat *) malloc(sizeof(GLfloat) * faces.size() * 3*3); // TODO: Make this more c++
    
    const float scale = .01f;
    int bufferIndex = 0;
    for(int i=0; i < faces.size(); i++) {
		for(int v=0; v<3; v++) {
			int vertexIndex = faces[i].vertex[v];
			//if(vertexIndex < 0 || vertexIndex >= vertices.size())
			//	LOGE("vertexIndex %d out of bounds (0, %d)", vertexIndex, vertices.size());
			struct Vertex vertex = vertices[vertexIndex];

			raptorVertices[bufferIndex++] = scale * vertex.coord.x;
			raptorVertices[bufferIndex++] = scale * vertex.coord.y;
			raptorVertices[bufferIndex++] = scale * vertex.coord.z;
		}
	}
	
	raptorVerticesSize = faces.size()*3;
    
    ///////////////////
    
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    
    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    
    // gProgram = createProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("depth_f.glsl"));

    if(!gProgram) {
        LOGE("Could not create program.");
        return;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
         gvPositionHandle);
    
    void *imageData = resourceCallback("raptor.jpg");
    textureUniform = glGetUniformLocation(gProgram, "Texture");
    
    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    free(imageData);
    
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return;
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

void RenderFrame() {
    if(!raptorVertices) {
        LOGE("raptorVertices undeclared");
        return;
    }
    static float delta = 0.01f;
    static float grey;
    grey += delta;
    if(grey > 1.0f || grey < 0.0f) {
        delta *= -1;
        grey += delta;
    }
    glClearColor(grey, .8f * grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(textureUniform, 0);
    
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, raptorVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, raptorVerticesSize);
    checkGlError("glDrawArrays");
}

#undef LOG_TAG
