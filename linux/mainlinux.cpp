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

void * stringResourceCB(const char * filename) {
    const char * path = "res/";
    char * filePath = (char *) malloc(strlen(path) + strlen(filename) + 1);
    strcpy(filePath, path);
    strcat(filePath, filename);
    ifstream file(filePath);
    free(filePath);
    if(!file.is_open()) {
        printf("Unable to open file %s", filename);
        return NULL;
    }
    
    string returnStr;
    getline(file, returnStr, '\0');
    file.close();
    
    return strdup(returnStr.c_str());
}

int main(int argc, char** argv) {

    // Initialize GLUT.
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
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

    SetResourceCallback(stringResourceCB);
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
