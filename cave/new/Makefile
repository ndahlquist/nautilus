include makefile.in

INCLUDE = -I$(OPENMESH_INCLUDE_DIR) -Iinclude/ -I$(EIGEN_DIR)
CPPFLAGS = -O3 -fPIC 
LDFLAGS = -O3 -framework GLUT -framework OpenGL
LIB = -lOpenMeshCored -lOpenMeshToolsd -Wl,-rpath,$(OPENMESH_LIB_DIR) 
TARGET = maze
OBJS = maze.o simplex.o

default: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -L$(OPENMESH_LIB_DIR) $(LIB) -o $(TARGET)

maze: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -L$(OPENMESH_LIB_DIR) $(LIB) -o animate

#renderSpline.o: renderSpline.cpp
#	$(CPP) -c $(CPPFLAGS) renderSpline.cpp -o renderSpline.o $(INCLUDE)

maze.o: maze.cpp
	$(CPP) -c $(CPPFLAGS) maze.cpp -o maze.o $(INCLUDE)

simplex.o: simplex/simplexnoise.cpp
	$(CPP) -c $(CPPFLAGS) simplex/simplexnoise.cpp -o simplex.o $(INCLUDE)

clean:
	rm $(OBJS) $(TARGET)
