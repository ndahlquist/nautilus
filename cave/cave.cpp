#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <Eigen/Core>
#include "mesh_definitions.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLUT/glut.h>
#include "spline.h"
#include "simplex/simplexnoise.h"
#include <time.h>
#include <stdlib.h>

using namespace Eigen;
using namespace std;
using namespace OpenMesh;

//static const double pi = 3.14159265359;
//static int kMsecsPerFrame = 100;
//static struct timeval last_idle_time;
vector<vector<Vec3f> > meshSpec;
OpenMesh::VPropHandleT<int> v_index;

bool leftDown = false, rightDown = false, middleDown = false;
int lastPos[2];
float cameraPos[4] = {0,0,4,1};
GLfloat light_position[] = {1, 1, 1, 0};
float shininess[] = {20.0};
float specular[] = {1.0, 1.0, 1.0, 1.0};

// Number of vertices and faces of the mesh
int n_vertices = 0, n_faces = 0;
int detailm = 50;
int detailn = 50;
int baseDetail = detailm/2;
double xDist = 0;
double yDist = 0;
double zDist = 0;

Vec3f up, pan;
bool showAxes = true;
int windowWidth = 640, windowHeight = 480;

// Function below modified from tutorial at
// http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/bezier-curves-and-surfaces-r1808
// (c) Jesper Tveit
void calculateV(Mesh* cave, int cPerRow, int perRow, int numV, Mesh::VertexHandle* vhandle) {
  Vec3f center(0,0,0);
  vector<Vec3f> base;
  vector<Vec3f> baseN;
  for (int s = 0; s < cPerRow - 3; s+=3){
    double Ax = meshSpec[0][s][0]; double Ay = meshSpec[0][s][1]; double Az = meshSpec[0][s][2];
    double Bx = meshSpec[0][s+1][0]; double By = meshSpec[0][s+1][1]; double Bz = meshSpec[0][s+1][2];
    double Cx = meshSpec[0][s+2][0]; double Cy = meshSpec[0][s+2][1]; double Cz = meshSpec[0][s+2][2];
    double Dx = meshSpec[0][s+3][0]; double Dy = meshSpec[0][s+3][1]; double Dz = meshSpec[0][s+3][2];

    double Ex = meshSpec[1][s][0]; double Ey = meshSpec[1][s][1]; double Ez =  meshSpec[1][s][2];
    double Fx = meshSpec[1][s+1][0]; double Fy =  meshSpec[1][s+1][1]; double Fz = meshSpec[1][s+1][2];
    double Gx = meshSpec[1][s+2][0]; double Gy =  meshSpec[1][s+2][1]; double Gz = meshSpec[1][s+2][2];
    double Hx = meshSpec[1][s+3][0]; double Hy =  meshSpec[1][s+3][1]; double Hz = meshSpec[1][s+3][2];

    double Ix = meshSpec[2][s][0]; double Iy = meshSpec[2][s][1]; double Iz = meshSpec[2][s][2];
    double Jx = meshSpec[2][s+1][0]; double Jy = meshSpec[2][s+1][1]; double Jz = meshSpec[2][s+1][2];
    double Kx = meshSpec[2][s+2][0]; double Ky = meshSpec[2][s+2][1]; double Kz = meshSpec[2][s+2][2];
    double Lx = meshSpec[2][s+3][0]; double Ly = meshSpec[2][s+3][1]; double Lz = meshSpec[2][s+3][2];

    double Mx = meshSpec[3][s][0]; double My = meshSpec[3][s][1]; double Mz = meshSpec[3][s][2];
    double Nx = meshSpec[3][s+1][0]; double Ny = meshSpec[3][s+1][1]; double Nz = meshSpec[3][s+1][2];
    double Ox = meshSpec[3][s+2][0]; double Oy = meshSpec[3][s+2][1]; double Oz = meshSpec[3][s+2][2];
    double Px = meshSpec[3][s+3][0]; double Py = meshSpec[3][s+3][1]; double Pz = meshSpec[3][s+3][2];
    // Vertices (maximum detail will now be 20·20 quads)
    double Xv[101][101];
    double Yv[101][101];
    double Zv[101][101]; 
    // Normal vectors
    double Xn[101][101];
    double Yn[101][101];
    double Zn[101][101];
    /* We will not actually draw a curved surface, but we will divide the
       surface into small quads and draw them. If the quads are small enough,
       it will appear as a curved surface. We will use a variable, detail, to
       define how many quads to use. Since the variables goes from 1.0 to 0.0
       we must change them by 1/detail from vertex to vertex. We will also
       store the vertices and the normal vectors in arrays and draw them in a
       separate loop */

    // Detail of 10 mean that we will calculate 11·11 vertices
    double changea = 1.0 / (double)detailm;
    double changec = 1.0 / (double)detailn;
   
    // Just making sure that the detail level is not set too high
    if(detailm > 100){
      detailm = 100;
    }
    if(detailn > 100){
      detailn = 100;
    }
    
    // Variables
    double a = 1.0;
    double b = 1.0 - a;
    double c = 1.0;
    double d = 1.0 - c;
    
    // Tangent vectors
    double Xta;
    double Yta;
    double Zta;
    
    double Xtc;
    double Ytc;
    double Ztc;
    /* Since we have two variables, we need two loops, we will change the
       a-variable from 1.0 to 0.0 by steps of 1/detail ( = change), and for each
       step we loop the c-variable from 1.0 to 0.0, thus creating a grid of
       points covering the surface. Note that we could have had separate detail
       levels for the a-variable and the c-variable if we wanted to */
    for(int i = 0; i <= detailm; i++)
      {
        for(int j = 0; j <= detailn; j++)
          {
            // First get the vertices
            Xv[i][j] = Ax*a*a*a*c*c*c   + Bx*3*a*a*a*c*c*d
              + Cx*3*a*a*a*c*d*d + Dx*a*a*a*d*d*d
              + Ex*3*a*a*b*c*c*c + Fx*9*a*a*b*c*c*d
              + Gx*9*a*a*b*c*d*d + Hx*3*a*a*b*d*d*d
              + Ix*3*a*b*b*c*c*c + Jx*9*a*b*b*c*c*d
              + Kx*9*a*b*b*c*d*d + Lx*3*a*b*b*d*d*d
              + Mx*b*b*b*c*c*c   + Nx*3*b*b*b*c*c*d
              + Ox*3*b*b*b*c*d*d + Px*b*b*b*d*d*d;
            
            Yv[i][j] = Ay*a*a*a*c*c*c   + By*3*a*a*a*c*c*d
              + Cy*3*a*a*a*c*d*d + Dy*a*a*a*d*d*d
              + Ey*3*a*a*b*c*c*c + Fy*9*a*a*b*c*c*d
              + Gy*9*a*a*b*c*d*d + Hy*3*a*a*b*d*d*d
              + Iy*3*a*b*b*c*c*c + Jy*9*a*b*b*c*c*d
              + Ky*9*a*b*b*c*d*d + Ly*3*a*b*b*d*d*d
              + My*b*b*b*c*c*c   + Ny*3*b*b*b*c*c*d
              + Oy*3*b*b*b*c*d*d + Py*b*b*b*d*d*d;
            
            Zv[i][j] = Az*a*a*a*c*c*c   + Bz*3*a*a*a*c*c*d
              + Cz*3*a*a*a*c*d*d + Dz*a*a*a*d*d*d
              + Ez*3*a*a*b*c*c*c + Fz*9*a*a*b*c*c*d
              + Gz*9*a*a*b*c*d*d + Hz*3*a*a*b*d*d*d
              + Iz*3*a*b*b*c*c*c + Jz*9*a*b*b*c*c*d
              + Kz*9*a*b*b*c*d*d + Lz*3*a*b*b*d*d*d
              + Mz*b*b*b*c*c*c   + Nz*3*b*b*b*c*c*d
              + Oz*3*b*b*b*c*d*d + Pz*b*b*b*d*d*d;
            
            // Then use the derived functions to get the tangent vectors
            Xta = Ax*3*a*a*c*c*c       + Bx*9*a*a*c*c*d
              + Cx*9*a*a*c*d*d       + Dx*3*a*a*d*d*d
              + Ex*3*(2*a-3*a*a)*c*c*c   + Fx*9*(2*a-3*a*a)*c*c*d
              + Gx*9*(2*a-3*a*a)*c*d*d   + Hx*3*(2*a-3*a*a)*d*d*d
              + Ix*3*(1-4*a+3*a*a)*c*c*c + Jx*9*(1-4*a+3*a*a)*c*c*d
              + Kx*9*(1-4*a+3*a*a)*c*d*d + Lx*3*(1-4*a+3*a*a)*d*d*d
              + Mx*3*(2*a-1-a*a)*c*c*c   + Nx*9*(2*a-1-a*a)*c*c*d
              + Ox*9*(2*a-1-a*a)*c*d*d   + Px*3*(2*a-1-a*a)*d*d*d;

            Yta = Ay*3*a*a*c*c*c       + By*9*a*a*c*c*d
              + Cy*9*a*a*c*d*d       + Dy*3*a*a*d*d*d
              + Ey*3*(2*a-3*a*a)*c*c*c   + Fy*9*(2*a-3*a*a)*c*c*d
              + Gy*9*(2*a-3*a*a)*c*d*d   + Hy*3*(2*a-3*a*a)*d*d*d
              + Iy*3*(1-4*a+3*a*a)*c*c*c + Jy*9*(1-4*a+3*a*a)*c*c*d
              + Ky*9*(1-4*a+3*a*a)*c*d*d + Ly*3*(1-4*a+3*a*a)*d*d*d
              + My*3*(2*a-1-a*a)*c*c*c   + Ny*9*(2*a-1-a*a)*c*c*d
              + Oy*9*(2*a-1-a*a)*c*d*d   + Py*3*(2*a-1-a*a)*d*d*d;

            Zta = Az*3*a*a*c*c*c       + Bz*9*a*a*c*c*d
              + Cz*9*a*a*c*d*d       + Dz*3*a*a*d*d*d
              + Ez*3*(2*a-3*a*a)*c*c*c   + Fz*9*(2*a-3*a*a)*c*c*d
              + Gz*9*(2*a-3*a*a)*c*d*d   + Hz*3*(2*a-3*a*a)*d*d*d
              + Iz*3*(1-4*a+3*a*a)*c*c*c + Jz*9*(1-4*a+3*a*a)*c*c*d
              + Kz*9*(1-4*a+3*a*a)*c*d*d + Lz*3*(1-4*a+3*a*a)*d*d*d
              + Mz*3*(2*a-1-a*a)*c*c*c   + Nz*9*(2*a-1-a*a)*c*c*d
              + Oz*9*(2*a-1-a*a)*c*d*d   + Pz*3*(2*a-1-a*a)*d*d*d;
            
            Xtc = Ax*3*a*a*a*c*c       + Bx*3*a*a*a*(2*c-3*c*c)
              + Cx*3*a*a*a*(1-4*c+3*c*c) + Dx*3*a*a*a*(-1+2*c-c*c)
              + Ex*9*a*a*b*c*c       + Fx*9*a*a*b*(2*c-3*c*c)
              + Gx*9*a*a*b*(1-4*c+3*c*c) + Hx*9*a*a*b*(-1+2*c-c*c)
              + Ix*9*a*b*b*c*c       + Jx*9*a*b*b*(2*c-3*c*c)
              + Kx*9*a*b*b*(1-4*c+3*c*c) + Lx*9*a*b*b*(-1+2*c-c*c)
              + Mx*3*b*b*b*c*c       + Nx*3*b*b*b*(2*c-3*c*c)
              + Ox*3*b*b*b*(1-4*c+3*c*c) + Px*3*b*b*b*(-1+2*c-c*c);
            
            Ytc = Ay*3*a*a*a*c*c       + By*3*a*a*a*(2*c-3*c*c)
              + Cy*3*a*a*a*(1-4*c+3*c*c) + Dy*3*a*a*a*(-1+2*c-c*c)
              + Ey*9*a*a*b*c*c       + Fy*9*a*a*b*(2*c-3*c*c)
              + Gy*9*a*a*b*(1-4*c+3*c*c) + Hy*9*a*a*b*(-1+2*c-c*c)
              + Iy*9*a*b*b*c*c       + Jy*9*a*b*b*(2*c-3*c*c)
              + Ky*9*a*b*b*(1-4*c+3*c*c) + Ly*9*a*b*b*(-1+2*c-c*c)
              + My*3*b*b*b*c*c       + Ny*3*b*b*b*(2*c-3*c*c)
              + Oy*3*b*b*b*(1-4*c+3*c*c) + Py*3*b*b*b*(-1+2*c-c*c);

            Ztc = Az*3*a*a*a*c*c       + Bz*3*a*a*a*(2*c-3*c*c)
              + Cz*3*a*a*a*(1-4*c+3*c*c) + Dz*3*a*a*a*(-1+2*c-c*c)
              + Ez*9*a*a*b*c*c       + Fz*9*a*a*b*(2*c-3*c*c)
              + Gz*9*a*a*b*(1-4*c+3*c*c) + Hz*9*a*a*b*(-1+2*c-c*c)
              + Iz*9*a*b*b*c*c       + Jz*9*a*b*b*(2*c-3*c*c)
              + Kz*9*a*b*b*(1-4*c+3*c*c) + Lz*9*a*b*b*(-1+2*c-c*c)
              + Mz*3*b*b*b*c*c       + Nz*3*b*b*b*(2*c-3*c*c)
              + Oz*3*b*b*b*(1-4*c+3*c*c) + Pz*3*b*b*b*(-1+2*c-c*c);
            
            // Cross the tangent vectors, put the result to the normal vector array
            // Note: I simplified -((Xta*Ztc)-(Xtc*Zta)) to (Xtc*Zta) - (Xta*Ztc)
            Xn[i][j] = (Yta*Ztc) - (Ytc*Zta);
            Yn[i][j] = (Xtc*Zta) - (Xta*Ztc);
            Zn[i][j] = (Xta*Ytc) - (Xtc*Yta);

            // Find length of normal vector
            double length = sqrt((Xn[i][j]*Xn[i][j])+(Yn[i][j]
                        *Yn[i][j])+(Zn[i][j]*Zn[i][j]));

            // Normalize (and prevent divide by zero error)
            if(length > 0)
              {
                length = 1.0/length;
                Xn[i][j] *= length;
                Yn[i][j] *= length;
                Zn[i][j] *= length;
              }

            //change the c-variable within the inner loop
            c -= changec;
            d  = 1.0 - c;
          }
        //change the a-variable outside the inner loop
        a -= changea;
        b  = 1.0 - a;

        // Reset the c-variable to make it ready for the inner loop again
        c = 1.0;
        d = 1.0 - c;
      }
    //add vertices to mesh
    for(int m = 0; m <= detailm; m++){
        for(int n = 0; n < detailn; n++)
          {
            int index = m * perRow + (detailn * (s/3)) + n;
            Vec3f point = Vec3f(Xv[m][n],Yv[m][n],Zv[m][n]);
            vhandle[index] = cave->add_vertex(point);
            if (abs(Xv[m][n]) > xDist) xDist = abs(Xv[m][n]);
            if (abs(Yv[m][n]) > yDist) yDist = abs(Yv[m][n]);
            if (abs(Zv[m][n]) > zDist) zDist = abs(Zv[m][n]);
            Vec3f norm(Xn[m][n],Yn[m][n],Zn[m][n]);
            cave->set_normal(vhandle[index], norm);
            cave->set_texcoord2D(vhandle[index], Vec2f(0,0));
            // cout << index << endl;
            if (index >= perRow * detailm){//base of the wall
              center += point;
              base.push_back(point);
            }
          }
    }
  }
  //calculate normal for base point
  int baseV = perRow * (baseDetail-1);
  Vec3f n(0,0,0);
  center = Vec3f(center[0]/perRow, center[1]/perRow, center[2]/perRow);
  for (int i = 0; i < perRow-1; i++){
    Vec3f faceN = (base[i+1] - center) % (base[i] - center);
    //faceN = faceN.normalize();
    // cout << faceN << endl;
    baseN.push_back(faceN);
    n += faceN;
  }
  Vec3f last = (base[0] - center) % (base[perRow-1] - center);
  n += last;
  baseN.push_back(last);
  vhandle[numV + baseV] = cave->add_vertex(center);
  n = Vec3f(n[0]/perRow, n[1]/perRow, n[2]/perRow);
  cave->set_normal(vhandle[numV + baseV], n.normalize());

  //calculate base
  double inc = 1/(double)baseDetail;
  Mesh::VertexHandle baseHandles[perRow*(baseDetail-1)];
  double delta = inc;
  for (int i = 0; i < baseDetail-1; i++){
    for (int j = 0; j < perRow; j++){
      double inv = 1-delta;
      Vec3f point = Vec3f(base[j][0]*inv + center[0]*delta, base[j][1]*inv + center[1]*delta, base[j][2]*inv + center[2]*delta);
      vhandle[n_vertices + i*perRow + j] = cave->add_vertex(point);
      cave->set_normal(vhandle[n_vertices + i*perRow + j], Vec3f(0,1,0));
    }
    delta += inc;
  }

  // for (int i = perRow * detailm; i < perRow * (detailm + 1); i++){
  //   Vec3f temp = cave->normal(vhandle[i]);
  //   temp += Vec3f(0,2/5,0);
  //   temp = temp.normalize();
  //   cave->set_normal(vhandle[i], temp);
  // }
  //Vec3f pointN = baseN[(j-1+perRow)%perRow] + baseN[j];
}

void init() {
  // Init lighting
  GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat l_ambient[] = {0.2, 0.2, 0.2, 1.0};

  glShadeModel(GL_FLAT);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, l_ambient);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  // set material
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void display() {
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0,0,windowWidth,windowHeight);
  
  float ratio = (float)windowWidth / (float)windowHeight;
  gluPerspective(50, ratio, 1, 1000); // 50 degree vertical viewing angle, zNear = 1, zFar = 1000
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2],
      pan[0], pan[1], pan[2], up[0], up[1], up[2]);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
  if (showAxes) {
    glBegin(GL_LINES);
    glLineWidth(1);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0); // x axis
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0); // y axis
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1); // z axis
    glEnd();
  }
  for (int i = 0; i < 4; i++){
    drawSpline(meshSpec[i]);
  }
  
  glutSwapBuffers();
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
  Vec3f curCamera(cameraPos[0],cameraPos[1],cameraPos[2]);
  Vec3f curCameraNormalized = curCamera.normalized();
  Vec3f right = up % curCameraNormalized;

  if (leftDown) {
    // Assume here that up vector is (0,1,0)
    Vec3f newPos = curCamera - 2 * ((float)dx/(float)windowWidth) * right
      + 2 * ((float)dy/(float)windowHeight) * up;
    newPos = newPos.normalized() * curCamera.length();
    
    up = up - (up | newPos) * newPos / newPos.sqrnorm();
    up.normalize();
    
    for (int i = 0; i < 3; i++) cameraPos[i] = newPos[i];
  }
  else if (rightDown) for (int i = 0; i < 3; i++) cameraPos[i] /= pow(1.1,dy*.1);
  else if (middleDown) {
    pan = -2*(float)((float)dx/(float)windowWidth) * right + 2*(float)((float)dy/(float)windowHeight) * up;
  }
  
  lastPos[0] = x;
  lastPos[1] = y;
  
  glutPostRedisplay();
}

void createCaveMesh(double w, double l, double h) {
  //meshSpec = readControlPts("keyframes.txt"); //use this to read control points from a file
  meshSpec = genControlPts(w, l, h);
  //set up mesh
  Mesh mesh;
  mesh.request_vertex_normals();
  //mesh.request_face_normals();
  mesh.request_vertex_texcoords2D();
  int cPerRow = meshSpec[0].size();
  int perRow = detailn*(cPerRow-1)/3;
  int perCol = detailm + 1;
  n_vertices = perRow * perCol;
  int baseVertices = perRow*(baseDetail-1);
  Mesh::VertexHandle vhandle[n_vertices + baseVertices + 1];
  vector<Mesh::VertexHandle> face_vhandles;
  genGrad(time(NULL)); //randomise noise
  calculateV(&mesh, cPerRow, perRow, n_vertices, vhandle);
  //walls
  for (int i = 0; i < perCol-1 + baseDetail-1; i++){
    for (int j = 0; j < perRow; j++){
      //face 1
      face_vhandles.clear();
      face_vhandles.push_back(vhandle[i*perRow + j]);
      face_vhandles.push_back(vhandle[(i+1)*perRow + j]);
      if (j == perRow - 1){
        face_vhandles.push_back(vhandle[i*perRow]);
      } else {
        face_vhandles.push_back(vhandle[i*perRow + j+1]);
      }
      mesh.add_face(face_vhandles);
      //face 2
      face_vhandles.clear();
      face_vhandles.push_back(vhandle[(i+1)*perRow + j]);
      if (j == perRow - 1){
        face_vhandles.push_back(vhandle[(i+1)*perRow]);
        face_vhandles.push_back(vhandle[i*perRow]);
      } else {
        face_vhandles.push_back(vhandle[(i+1)*perRow + j+1]);
        face_vhandles.push_back(vhandle[i*perRow + j+1]);
      }
      mesh.add_face(face_vhandles);
    }
  }

  //base
  for (int i = n_vertices + baseVertices - perRow; i < n_vertices + baseVertices -1; i++){
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[i]);
    face_vhandles.push_back(vhandle[n_vertices + baseVertices]);
    face_vhandles.push_back(vhandle[i+1]);
    mesh.add_face(face_vhandles);
  }

  //add last face (wraps around to first vertex)
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[n_vertices + baseVertices -1]);
  face_vhandles.push_back(vhandle[n_vertices + baseVertices]);
  face_vhandles.push_back(vhandle[n_vertices + baseVertices - perRow]);
  mesh.add_face(face_vhandles);

  //add noise
  double scale = max( max(xDist, yDist), zDist);
  //mesh.update_normals(); 
  for (Mesh::ConstVertexIter vIt = mesh.vertices_begin(); vIt != mesh.vertices_end(); ++vIt){
    Vec3f vert = mesh.point(vIt);
    double effect = 0.5 * octave_noise_3d(5.0, 0.5, 1.3, vert[0]/scale, vert[1]/scale, vert[2]/scale);
    Vec3f shift(effect*mesh.normal(vIt)[0], effect*mesh.normal(vIt)[1], effect*mesh.normal(vIt)[2]);
    mesh.set_point(vIt, mesh.point(vIt) - shift);
  }

  mesh.update_normals();
  //mesh.release_face_normals();
  IO::Options wopt;
  wopt += IO::Options::VertexNormal;
  wopt += IO::Options::VertexTexCoord;
  if (!OpenMesh::IO::write_mesh(mesh, "meshfile.obj", wopt)) 
  {
    std::cerr << "write error\n";
    exit(1);
  }
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q' || key == 'Q') exit(0);
  else if (key == 'a' || key == 'A') showAxes = !showAxes;
}

void reshape(int width, int height) {
  windowWidth = width;
  windowHeight = height;
  glutPostRedisplay();
}

int main(int argc, char** argv) {
  up = Vec3f(0, 1, 0);
  pan = Vec3f(0, 0, 0);
  //Vec3f center = Vec3f(0, 0, 0);


  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  glutInitWindowSize(windowWidth, windowHeight); 
  glutCreateWindow(argv[0]);
  
  init();
  if (argc == 4){
    createCaveMesh(atof(argv[1]), atof(argv[2]), atof(argv[3]));
  } else {
     createCaveMesh(5, 8, 4);
  }

  glutDisplayFunc(display);
  glutMotionFunc(mouseMoved);
  glutMouseFunc(mouse);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);

  glutMainLoop();
  
  return 0;
}
