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

double tileSize = 1; 
int maxDistort = 12;
OpenMesh::VPropHandleT<int> v_index;

// Number of vertices and faces of the mesh
int n_vertices = 0, n_faces = 0;


void randomOrder(vector<Wall>* arr){
  srand(time(NULL));
  for (int i = 0; i < arr->size(); i++){
    int swap = rand() % arr->size();
    Wall temp = (*arr)[i];
    (*arr)[i] = (*arr)[swap];
    (*arr)[swap] = temp;
  }
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
        double distortx = (double)(rand() % maxDistort)/(1.8*maxDistort/tileSize);
        double distorty = (double)(rand() % maxDistort)/(1.8*maxDistort/tileSize);
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
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              face_vhandles.push_back(mainVert[k][i][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              face_vhandles.push_back(mainVert[k][i][j]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //top
            if (i == height-1 || !cells[k][i+1][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
          }
          if(type == FC || type == CORE || type == XWALL){
            //left
            if (k == 0 || !cells[k-1][i][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              face_vhandles.push_back(mainVert[k][i+1][j]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //right
            if (k == length-1 || !cells[k+1][i][j].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k+1][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
          }
          if(type == FC || type == CORE || type == ZWALL){
            //close
            if (j == 0 || !cells[k][i][j-1].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j]);
              face_vhandles.push_back(mainVert[k+1][i+1][j]);
              face_vhandles.push_back(mainVert[k+1][i][j]);
              maze.add_face(face_vhandles);
              n_faces += 2;
            }
            //far
            if (j == width-1 || !cells[k][i][j+1].isWall()){
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
              face_vhandles.push_back(mainVert[k][i+1][j+1]);
              maze.add_face(face_vhandles);
              face_vhandles.clear();
              face_vhandles.push_back(mainVert[k][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i][j+1]);
              face_vhandles.push_back(mainVert[k+1][i+1][j+1]);
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

int main(int argc, char** argv){
  if (argc > 3){
    if (argc == 5) {
      generateMaze(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]));
    } else {
      generateMaze(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1);
    }
  } else {
    generateMaze(3, 4, 3, 1);
  }
  return 0;
}
