#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <Eigen/Core>
#include "mesh_definitions.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLUT/glut.h>
#include "simplex/simplexnoise.h"
#include <time.h>
#include <stdlib.h>
#include "chamber.h"

using namespace Eigen;
using namespace std;
using namespace OpenMesh;

//static const double pi = 3.14159265359;
//static int kMsecsPerFrame = 100;
//static struct timeval last_idle_time;

double tileSize = 1; 
int maxDistort = 12;
OpenMesh::VPropHandleT<int> v_index;

bool leftDown = false, rightDown = false, middleDown = false;
int lastPos[2];
float cameraPos[4] = {0,0,4,1};
GLfloat light_position[] = {1, 1, 1, 0};
float shininess[] = {20.0};
float specular[] = {1.0, 1.0, 1.0, 1.0};

// Number of vertices and faces of the mesh
int n_vertices = 0, n_faces = 0;
double xDist = 0;
double yDist = 0;
double zDist = 0;

Vec3f up, pan;
bool showAxes = true;
int windowWidth = 640, windowHeight = 480;

// Store the indices of even/odd vertices of each old face
typedef struct face_info {
  int even[3];
  int odd[3];
}FaceInfo;

string getEdgeKey(int e1, int e2) {
  ostringstream oss;
  if (e1 < e2) 
    oss << e1 << "," << e2;
  else 
    oss << e2 << "," << e1;
  return oss.str();
}

void randomOrder(vector<Wall>* arr){
  srand(time(NULL));
  for (int i = 0; i < arr->size(); i++){
    int swap = rand() % arr->size();
    Wall temp = (*arr)[i];
    (*arr)[i] = (*arr)[swap];
    (*arr)[swap] = temp;
  }
}

void loopSubdivision(Mesh* mesh) {
  FaceInfo new_faces[n_faces];
  map<string, int> edge_to_odd_v;
  int odd_v_idx = n_vertices;
  int f_idx = 0;
  for (Mesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it, ++f_idx) {
    int i = 0;
    for (Mesh::FaceVertexIter fv_it = mesh->fv_iter(f_it.handle()); fv_it; ++fv_it, ++i) {
      new_faces[f_idx].even[i] = mesh->property(v_index, fv_it.handle());
      Vec3f p = mesh->point(fv_it.handle());
    }
    for (int i = 0; i < 3; ++i) {
      int e1 = new_faces[f_idx].even[i];
      int e2 = new_faces[f_idx].even[(i + 1) % 3];
      string key = getEdgeKey(e1, e2);
      if (edge_to_odd_v.find(key) == edge_to_odd_v.end()) {
  edge_to_odd_v[key] = odd_v_idx;
  ++odd_v_idx;
      } 
      new_faces[f_idx].odd[i] = edge_to_odd_v[key];
    }
  }
  // Create the new mesh and update its vertex/face data
  Mesh new_mesh;
  new_mesh.request_face_normals();
  new_mesh.request_vertex_normals();
  new_mesh.request_vertex_texcoords2D();
  Mesh::VertexHandle vhandle[odd_v_idx];
  Vec3f vertices[odd_v_idx];
  new_mesh.add_property(v_index);
  // Update even vertices
  for (Mesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
    Vec3f sum(0, 0, 0);
    int n_neighbors = 0;
    for (Mesh::VertexVertexIter vv_it = mesh->vv_iter(v_it.handle()); vv_it; ++vv_it) {
      sum += mesh->point(vv_it.handle());
      ++n_neighbors;
    }
    float beta;
    if (n_neighbors > 3) {
      beta = 3.0 / ((float)n_neighbors * 8.0); 
    } else {
      beta = 3.0 / 16.0;
    }
    sum *= beta;
    float ratio = 1.0 - n_neighbors * beta;
    Vec3f p = ratio * mesh->point(v_it.handle());
    sum += p;
    int v_idx = mesh->property(v_index, v_it.handle());
    vertices[v_idx] = sum;
  }
  // Update odd vertices 
  for (Mesh::EdgeIter e_it = mesh->edges_begin(); e_it != mesh->edges_end(); ++e_it) {
    Mesh::HalfedgeHandle he_h = mesh->halfedge_handle(e_it.handle(), 0);
    Vec3f sum(0, 0, 0);
    sum += mesh->point(mesh->to_vertex_handle(he_h)); 
    sum += mesh->point(mesh->from_vertex_handle(he_h));
    int e1 = mesh->property(v_index, mesh->to_vertex_handle(he_h));
    int e2 = mesh->property(v_index, mesh->from_vertex_handle(he_h));
    string key = getEdgeKey(e1, e2);
    if (mesh->is_boundary(he_h)) {
      sum /= 2.0;  
    } else {
      for (int i = 0; i < 2; ++i) {
  Mesh::HalfedgeHandle he_h = mesh->halfedge_handle(e_it.handle(), i);
  Mesh::FaceHandle f_h = mesh->face_handle(he_h);
  for (Mesh::FaceVertexIter fv_it = mesh->fv_iter(f_h); fv_it; ++fv_it) {
    sum += mesh->point(fv_it.handle());
  }
      }
      sum /= 8.0;
    }
    if (edge_to_odd_v.find(key) == edge_to_odd_v.end())
      cout << "key: " << key << " not found, shouldn't happen!!!" << endl;
    vertices[edge_to_odd_v[key]] = sum;
  }

  // Create new mesh
  n_vertices = odd_v_idx;
  for (int i = 0; i < n_vertices; ++i) {
    vhandle[i] = new_mesh.add_vertex(vertices[i]);
    new_mesh.property(v_index, vhandle[i]) = i;
    // new_mesh.property(texture, vhandle[i]) = Vec2f(0, 0);
    new_mesh.set_texcoord2D(vhandle[i], Vec2f(0,0));
  }
  vector<Mesh::VertexHandle> face_vhandles;
  for (int i = 0; i < n_faces; ++i) {
    // Add four faces for each old face
    FaceInfo* face = new_faces + i;
    // Face 1
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[face->even[0]]);
    face_vhandles.push_back(vhandle[face->odd[0]]);
    face_vhandles.push_back(vhandle[face->odd[2]]);
    new_mesh.add_face(face_vhandles);
    // Face 2
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[face->odd[0]]);
    face_vhandles.push_back(vhandle[face->even[1]]);
    face_vhandles.push_back(vhandle[face->odd[1]]);
    new_mesh.add_face(face_vhandles);
    // Face 3
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[face->odd[2]]);
    face_vhandles.push_back(vhandle[face->odd[1]]);
    face_vhandles.push_back(vhandle[face->even[2]]);
    new_mesh.add_face(face_vhandles);
    // Face 4
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[face->odd[2]]);
    face_vhandles.push_back(vhandle[face->odd[0]]);
    face_vhandles.push_back(vhandle[face->odd[1]]);
    new_mesh.add_face(face_vhandles);
  }
  n_faces *= 4;
  new_mesh.update_normals();
  mesh = &new_mesh;
}

void generateMesh(int width, int length, int height, vector <vector <vector<Chamber> > > cells) { //note that this is lwh of actual cells (including walls)
  Mesh maze;
  srand(time(NULL));
  maze.request_face_normals();
  maze.request_vertex_normals();
  maze.request_vertex_texcoords2D();
  //maze.add_property(v_index);
  //GENERATE MAIN VERTICES THAT WILL ANCHOR THE WALLS
  Mesh::VertexHandle mainVert[length+1][height+1][width+1];
  for (int i = 0; i <= height; i++){
    for (int j = 0; j <= width; j++){
      for (int k = 0; k <= length; k++){
        double distortx = (double)(rand() % maxDistort)/(2*maxDistort/tileSize);
        double distorty = (double)(rand() % maxDistort)/(2*maxDistort/tileSize);
        double distortz = (double)(rand() % maxDistort)/(2*maxDistort/tileSize);
        if (k != length-1 && (k%2==1 || k == 0)) distortx = -distortx;
        if (j != width-1 && (j%2==1 || j == 0)) distortz = -distortz;
        if (i != height-1 && (i%2==1 || i == 0)) distorty = -distorty;
        Vec3f point(k * tileSize + distortx, i * tileSize + distorty, j * tileSize + distortz);
        mainVert[k][i][j] = maze.add_vertex(point);
        //maze.property(v_index, mainVert[k][i][j]) = i*height + j*width + k;
        //maze.set_texcoord2D(mainVert[k][i][j], Vec2f(0,0));
      }
    }
  }
  //LINK UP MESH
  vector<Mesh::VertexHandle> face_vhandles;
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      for (int k = 0; k < length; k++){
        if (cells[k][i][j].isWall()){ //if it is a wall
          int type = cells[k][i][j].type();
          if (type == FC || type == YWALL){
            //bottom
            if (i == 0 || !cells[k][i-1][j].isWall() ){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //top
            if (i == height-1 || !cells[k][i+1][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
          }
          if(type == FC || type == CORE || type == XWALL){
            //left
            if (k == 0 || !cells[k-1][i][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              face_vhandles.push_back(mainVert[k][i][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //right
            if (k == length-1 || !cells[k+1][i][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
          }
          if(type == FC || type == CORE || type == ZWALL){
            //close
            if (j == 0 || !cells[k][i][j-1].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              face_vhandles.push_back(mainVert[k][i+1][j]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //far
            if (j == width-1 || !cells[k][i][j+1].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
          }

        }
      }
    }
  }
  
  // subdivide
  Loop sDivide;
  sDivide.attach(maze);
  bool success = sDivide(3);
  sDivide.detach();
  maze.update_normals();

  double scale = max(max(length, width), height)*tileSize;
  for (Mesh::ConstVertexIter vIt = maze.vertices_begin(); vIt != maze.vertices_end(); ++vIt){
    Vec3f vert = maze.point(vIt);
    double effect = 0.4*tileSize * octave_noise_3d(6.0, .75, 2, vert[0]/scale, vert[1]/scale, vert[2]/scale);
    Vec3f shift(effect*maze.normal(vIt)[0], effect*maze.normal(vIt)[1], effect*maze.normal(vIt)[2]);
    maze.set_point(vIt, vert - shift);
    maze.set_texcoord2D(vIt, Vec2f(0,0));
  }
  maze.update_normals();

  //WRITE OBJ FILE
  IO::Options wopt;
  wopt += IO::Options::VertexNormal;
  wopt += IO::Options::VertexTexCoord;
  if (!OpenMesh::IO::write_mesh(maze, "maze.obj", wopt)) 
  {
    std::cerr << "write error\n";
    exit(1);
  }

}

void generateMaze(int width, int length, int height, double scale){
  //Use Kruskal's algorithm to get a maze starting with a 3d set of walls (i.e. cubes)
  tileSize = scale;
	width = width*2+1;
	length = length*2+1;
	height = height*2+1;
  n_vertices = (width+1)*(length+1)*(height+1);
	vector<Wall> mazeWalls; //these are only walls which may be removed
	//Chamber cells[length][height][width];
  vector <vector <vector<Chamber> > > cells(length, vector<vector<Chamber> >(height, vector<Chamber>(width) ) );
  //GENERATE ORIGINAL GRID OF WALLS AND CHAMBERS
	for (int i = 0; i < height; i++){ //for each level (Y direction)
  	for(int j = 0; j < width; j++){ //for each row (Z direction)
  		for(int k = 0; k < length; k++){ //for each element in row (X direction)
        cells[k][i][j].parent = &(cells[k][i][j]);
        // if (i == 0 || j == 0 || k == 0 || i == height-1 || j == width-1 || k == length-1){
        //   cells[k][i][j].setType(BORDER);
        //   continue;
        // }
  			if (i%2 == 0){	//even level 
  				if (j%2 == 1 && k%2 == 1){ //odd-odd cells are floor/ceilings that can be removed
  					Wall w = Wall(k, i, j, YWALL); 
            cells[k][i][j].setType(YWALL);
            if (i == 0 || j == 0 || k == 0 || i == height-1 || j == width-1 || k == length-1) continue;
            mazeWalls.push_back(w);
  				}	else { //other cells are floor/ceilings that can't be removed
            cells[k][i][j].setType(FC);
          }
  			} else { //odd level
  				if (k%2 == 1){ 
            if (j%2 == 0){//odd x, even z - this is a wall
              Wall w = Wall(k, i, j, ZWALL); 
              cells[k][i][j].setType(ZWALL);
              if (i == 0 || j == 0 || k == 0 || i == height-1 || j == width-1 || k == length-1) continue;
              mazeWalls.push_back(w);
            } //else: odd x, odd z - this is an open chamber aka default
  				} else {
            if (j%2 == 1){ //even x, odd z - this is a wall
              Wall w = Wall(k, i, j, XWALL); 
              cells[k][i][j].setType(XWALL);
              if (i == 0 || j == 0 || k == 0 || i == height-1 || j == width-1 || k == length-1) continue;
              mazeWalls.push_back(w);
            } else { //even x, even z - this is a core wall
              cells[k][i][j].setType(CORE);
            }
          }
  			}
  		}
  	}
	}
  // cout << mazeWalls.size() << endl;
  //SHUFFLE WALLS
  randomOrder(&mazeWalls);
  //DELETE WALLS UNTIL CONNECTED
  for (int i = 0; i < mazeWalls.size(); i++){
    Wall w = mazeWalls[i];
    Chamber* one = cells[w.ax][w.ay][w.az].getRoot();
    Chamber* two = cells[w.bx][w.by][w.bz].getRoot();
    if (one != two){ //if the wall separates unconnected chambers
      //combine chambers
      cUnion(&cells[w.ax][w.ay][w.az], &cells[w.bx][w.by][w.bz]);
      cells[w.sx][w.sy][w.sz].setType(NOTWALL);
    }
  }
  
  //FOR TESTING PURPOSES: PRINT MAZE IN LAYERS
  for (int i = 0; i < height; i++){
    cout << endl << "Level " << i << endl;
    for (int j = 0; j < width; j++){
      for (int k = 0; k < length; k++){
        if (cells[k][i][j].isWall()){
          int type = cells[k][i][j].type();
          switch(type){
            case FC: 
              cout << "F";
              break;
            case YWALL:
              cout << "Y";
              break;
            case XWALL:
              cout << "X";
              break;
            case ZWALL:
              cout << "Z";
              break;
            case CORE:
              cout << "C";
              break;
            case BORDER:
              cout << "B";
              break;
            default: 
              cout << type;
              break;
          }
        } else {
          cout << "_";
        }
      }
      cout << endl;
    }
  }

  generateMesh(width, length, height, cells);

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

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q' || key == 'Q') exit(0);
  else if (key == 'a' || key == 'A') showAxes = !showAxes;
}

void reshape(int width, int height) {
  windowWidth = width;
  windowHeight = height;
  glutPostRedisplay();
}

int main(int argc, char** argv){
  up = Vec3f(0, 1, 0);
  pan = Vec3f(0, 0, 0);
  Vec3f center = Vec3f(0, 0, 0);

  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  glutInitWindowSize(windowWidth, windowHeight); 
  glutCreateWindow(argv[0]);
  
  init();
  if (argc > 3){
    if (argc = 4) {
      generateMaze(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]));
    } else {
      generateMaze(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1);
    }
  } else {
    generateMaze(3, 4, 3, 1);
  }
  // glutDisplayFunc(display);
  // glutMotionFunc(mouseMoved);
  // glutMouseFunc(mouse);
  // glutReshapeFunc(reshape);
  // glutKeyboardFunc(keyboard);

  //glutMainLoop();
  
  return 0;
}
