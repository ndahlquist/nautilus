#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <vector>
#include <map>
#include <GLUT/GLUT.h>
#include "Fluid.h"
#include "Eigen/Geometry"
using namespace Eigen;

enum DrawType{GLUT,ODE};

bool leftDown = false, rightDown = false, middleDown = false;
int lastPos[2];
float cameraPos[4] = {3,5,5,1};
Vector3f pan, up;
int windowWidth = 640, windowHeight = 480;
GLfloat ambientlight[] = {0.3,0.3,0.3,1.0};
GLfloat ambient[] = {0.6,0.6,0.6,1};
GLfloat ambient_floor[] = {0.,0.,0.,1.0};
GLfloat spotlight[] = {1.0, 1.0,1.0,1.0};
GLfloat position[] = {0, 20, 0,1};
GLfloat spot_position[] = {5,50,0,1.0};
float specular[] = { 1.0, 1.0, 1.0, 1.0 };
float shininess[] = { 50.0 };
bool showSurface = true, showAxes = true, flatShading = false, showTexture = true, showLighting=true, showMesh = true;


Fluid *Water;


//GLUT
void display(){
    glClearColor(1.0,1.0,1.0,0.0);
	glShadeModel(GL_SMOOTH);
  
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
    

    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0,0,windowWidth,windowHeight);
    
    float ratio = (float)windowWidth / (float)windowHeight;
    gluPerspective(50, ratio, 0.1, 1000); // 50 degree vertical viewing angle, zNear = 1, zFar = 1000
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);
    
    //glScalef(3, 3, 3);
    glTranslatef(-1, 0, 0);
	
   
    
    //glColor3f(1.0, 0.0, 0.0);
    Water->RenderFrame();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.f,0,0);
    glBegin(GL_POLYGON);
    glVertex3f(0,0,0);
    glVertex3f(CELL_WIDTH*Cell_NUM_X,0,0);
    glVertex3f(CELL_WIDTH*Cell_NUM_X, CELL_WIDTH*Cell_NUM_Y, 0);
    glVertex3f(0,CELL_WIDTH*Cell_NUM_Y,0);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3f(0,0,0);
    glVertex3f(0,CELL_WIDTH*Cell_NUM_Y,0);
    glVertex3f(0, CELL_WIDTH*Cell_NUM_Y, CELL_WIDTH*Cell_NUM_Z);
    glVertex3f(0,0,CELL_WIDTH*Cell_NUM_Z);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3f(0,0,0);
    glVertex3f(0,0,CELL_WIDTH*Cell_NUM_Z);
    glVertex3f(CELL_WIDTH*Cell_NUM_X, 0, CELL_WIDTH*Cell_NUM_Z);
    glVertex3f(CELL_WIDTH*Cell_NUM_X,0,0);
    glEnd();
       
	glutSwapBuffers();

}

void reshape( int w, int h ){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.02,1.02,-0.02,1.02,-10.0,10.0 );//指定了所使用的坐标系统
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
}
void idle(){
	
}
void timer(int p){

	Water->Update();
	glutPostRedisplay();
	glutTimerFunc(2,timer,0);
}
void WaterInit(){
	
	//Init()
	Water = new Fluid();
	
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) leftDown = (state == GLUT_DOWN);
    else if (button == GLUT_RIGHT_BUTTON) rightDown = (state == GLUT_DOWN);
    else if (button == GLUT_MIDDLE_BUTTON) middleDown = (state == GLUT_DOWN);
    
    lastPos[0] = x;
    lastPos[1] = y;
}

void mouseMoved(int x, int y) {
    int dx = x - lastPos[0];
    int dy = y - lastPos[1];
    Vector3f curCamera(cameraPos[0],cameraPos[1],cameraPos[2]);
    Vector3f curCameraNormalized = curCamera;
    curCameraNormalized.normalize();
    Vector3f right = up.cross(curCameraNormalized);
    
    if (leftDown) {
        // Assume here that up vector is (0,1,0)
        Vector3f newPos = curCamera - (float)((float)dx/(float)windowWidth) * right + (float)((float)dy/(float)windowHeight) * up;
        newPos.normalize();
        newPos = newPos * curCamera.norm();
        
        up = up - up.dot(newPos) * newPos / newPos.squaredNorm();
        up.normalize();
        
        cameraPos[0] = newPos[0];
        cameraPos[1] = newPos[1];
        cameraPos[2] = newPos[2];
        
    }
    else if (rightDown) for (int i = 0; i < 3; i++) cameraPos[i] *= pow(1.1,dy*.1);
    else if (middleDown) {
        pan += -2*(float)((float)dx/(float)windowWidth) * right + 2*(float)((float)dy/(float)windowHeight) * up;
    }
    
    
    lastPos[0] = x;
    lastPos[1] = y;
    
    glutPostRedisplay();
}


void keyboard(unsigned char key, int x, int y) {
    
    if (key == 'q' || key == 'Q') exit(0);
    glutPostRedisplay();
}


int main(int argc,char **argv){
	
    WaterInit();
    
    up = Vector3f(0,1,0);
    pan = Vector3f(0,0,0);
    
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	glutInitWindowSize( windowWidth, windowHeight );
	glutInitWindowPosition( 0, 0 );
	glutCreateWindow( "FLUID" );
	glutDisplayFunc(display);
	glutReshapeFunc( reshape );
    glutMotionFunc(mouseMoved);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
	glutTimerFunc( 2, timer, 0 );
	glutIdleFunc( idle );
	glutMainLoop();
	return 0;
}
		
		
		

	