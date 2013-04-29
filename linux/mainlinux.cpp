
#include "stglew.h"
#include "st.h"
#include <stdio.h>

#include "common.h"

// Window size, kept for screenshots
static int gWindowSizeX = 200;
static int gWindowSizeY = 200;

//
// Display the output image from our vertex and fragment shaders
//
void DisplayCallback() {
    RenderFrame();
    glutSwapBuffers();
}

//
// Reshape the window and record the size so
// that we can use it for screenshots.
//
void ReshapeCallback(int w, int h) {
	gWindowSizeX = w;
    gWindowSizeY = h;
    Setup(gWindowSizeX, gWindowSizeY);

  /*glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	// Set up a perspective projection
    float aspectRatio = (float) gWindowSizeX / (float) gWindowSizeY;
	gluPerspective(30.0f, aspectRatio, .1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();*/
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
    case 's': {
            //
            // Take a screenshot, and save as screenshot.jpg
            //
            STImage* screenshot = new STImage(gWindowSizeX, gWindowSizeY);
            screenshot->Read(0,0);
            screenshot->Save("screenshot.jpg");
            delete screenshot;
        }
        break;
    case 'r':
        //resetCamera();
        break;
    case 't':
        //teapot = !teapot;
        break;
	case 'q':
		exit(0);
    default:
        break;
    }

    // glutPostRedisplay();
}

/**
 * Mouse event handler
 */
void MouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
        //gMouseButton = button;
    } else {
        //gMouseButton = -1;
    }
    
    if (state == GLUT_UP) {
        //gPreviousMouseX = -1;
        //gPreviousMouseY = -1;
    }
}

/**
 * Mouse active motion callback (when button is pressed)
 /*/
void MouseMotionCallback(int x, int y) {
    /*if (gPreviousMouseX >= 0 && gPreviousMouseY >= 0)
    {
        //compute delta
        float deltaX = x-gPreviousMouseX;
        float deltaY = y-gPreviousMouseY;
        gPreviousMouseX = x;
        gPreviousMouseY = y;
        
        float zoomSensitivity = 0.2f;
        float rotateSensitivity = 0.5f;
        
        //orbit or zoom
        if (gMouseButton == GLUT_LEFT_BUTTON)
        {
            AdjustCameraAzimuthBy(-deltaX*rotateSensitivity);
            AdjustCameraElevationBy(-deltaY*rotateSensitivity);
            
        } else if (gMouseButton == GLUT_RIGHT_BUTTON)
        {
            STVector3 zoom(0,0,deltaX);
            AdjustCameraTranslationBy(zoom * zoomSensitivity);
        }
        
    } else
    {
        gPreviousMouseX = x;
        gPreviousMouseY = y;
    }*/
    
}

int main(int argc, char** argv) {
	/*if (argc != 5)
		usage();

	vertexShader   = std::string(argv[1]);
	fragmentShader = std::string(argv[2]);
	lightProbe     = std::string(argv[3]);
	normalMap      = std::string(argv[4]);*/

    // Initialize GLUT.
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("CS148 Assignment 7");
    
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

    // Be sure to initialize GLUT (and GLEW for this assignment) before
    // initializing your application.


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
