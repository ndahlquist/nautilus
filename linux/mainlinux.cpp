// mainlinux.cpp
// nativeGraphics

#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include "common.h"
#include "log.h"
#include "jpegHelper.h"
#include "pngHelper.h"

using namespace std;

// Window size, kept for screenshots
static int gWindowSizeX = 200;
static int gWindowSizeY = 200;

// Display the output image from our vertex and fragment shaders
void DisplayCallback() {
    RenderFrame();
    glutSwapBuffers();
}

// Reshape the window and record the size so
// that we can use it for screenshots.
void ReshapeCallback(int w, int h) {
	gWindowSizeX = w;
    gWindowSizeY = h;
    Setup(gWindowSizeX, gWindowSizeY);
}

void SpecialKeyCallback(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            //AdjustCameraTranslationBy(STVector3(-0.2,0,0));
            break;
        case GLUT_KEY_RIGHT:
            //AdjustCameraTranslationBy(STVector3(0.2,0,0));
            break;
        case GLUT_KEY_DOWN:
            //AdjustCameraTranslationBy(STVector3(0,-0.2,0));
            break;
        case GLUT_KEY_UP:
            //AdjustCameraTranslationBy(STVector3(0,0.2,0));
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void KeyCallback(unsigned char key, int x, int y) {
    switch(key) {
	case 'q':
		exit(0);
    default:
        break;
    }

    // glutPostRedisplay();
}

void MouseCallback(int button, int state, int x, int y) {
    if(state == GLUT_DOWN)
        PointerDown((float) x / (float)gWindowSizeX, (float) y / (float)gWindowSizeY);
    if(state == GLUT_UP)
        PointerUp((float) x / (float)gWindowSizeX, (float) y / (float)gWindowSizeY);
}

void MouseMotionCallback(int x, int y) {
    PointerMove((float) x / (float)gWindowSizeX, (float) y / (float)gWindowSizeY);
}

bool checkExt(const char * cfileName, const char * ext) {
    char * fileName = strdup(cfileName);
    strtok(fileName, ".");
    char * fileExt = strtok(NULL, ".");
    bool match = strcmp(fileExt, ext) == 0;
    free(fileName);
    return match;
}

char * stringResourceCallback(const char * fileName) {
    const char * path = "../res/raw/";
    char * filePath = (char *) malloc(strlen(path) + strlen(fileName) + 1);
    strcpy(filePath, path);
    strcat(filePath, fileName);
    ifstream file(filePath);
    free(filePath);
    if(!file.is_open()) {
        printf("Unable to open file %s\n", fileName);
        return NULL;
    }
    
    string returnStr;
/* OpenGL ES requires precision identifiers in shaders, while regular OpenGL
   while not compile with precision specifiers. We get around this by skippinng
   the first line of GLSL files. */
    if(checkExt(fileName, "glsl"))
        getline(file, returnStr);
    getline(file, returnStr, '\0');
    file.close();
    
    return strdup(returnStr.c_str());
}

void * ResourceCallback(const char * fileName, int * width, int * height) {
    if(checkExt(fileName, "jpg") || checkExt(fileName, "jpeg")) {
        if(width && height)
            return jpegResourceCallback(fileName, *width, *height);
        LOGI("You should probably have passed width and height here.");
        int temp1, temp2;
        return jpegResourceCallback(fileName, temp1, temp2);
    }
    if(checkExt(fileName, "png")) {
        if(width && height)
            return pngResourceCallback(fileName, *width, *height);
        LOGI("You should probably have passed width and height here.");
        int temp1, temp2;
        return pngResourceCallback(fileName, temp1, temp2);
    }
    if(width)
        *width = -1;
    if(height)
        *height = -1;
    return stringResourceCallback(fileName);
}

int main(int argc, char** argv) {

    // Initialize GLUT.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("nativeGraphics");
    
    // Initialize GLEW.
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
			   "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
				   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }

    SetResourceCallback(ResourceCallback);
    Setup(gWindowSizeX, gWindowSizeY);

    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutSpecialFunc(SpecialKeyCallback);
    glutKeyboardFunc(KeyCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MouseMotionCallback);
    glutIdleFunc(DisplayCallback);

    glutMainLoop();

    // Cleanup code should be called here.

    return 0;
}
