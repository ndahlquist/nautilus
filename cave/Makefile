include makefile.in

INCLUDE = -I$(OPENMESH_INCLUDE_DIR) -Iinclude/ -I$(EIGEN_DIR)
CPPFLAGS = -O3 -fPIC -DEIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET 
LDFLAGS = -O3 -framework GLUT -framework OpenGL
LIB = -lOpenMeshCored -lOpenMeshToolsd -Wl,-rpath,$(OPENMESH_LIB_DIR) 
TARGET = cave
OBJS = cave.o simplex.o

default: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -L$(OPENMESH_LIB_DIR) $(LIB) -o $(TARGET)

cave: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -L$(OPENMESH_LIB_DIR) $(LIB) -o animate

#renderSpline.o: renderSpline.cpp
#	$(CPP) -c $(CPPFLAGS) renderSpline.cpp -o renderSpline.o $(INCLUDE)

cave.o: cave.cpp
	$(CPP) -c $(CPPFLAGS) cave.cpp -o cave.o $(INCLUDE)

simplex.o: simplex/simplexnoise.cpp
	$(CPP) -c $(CPPFLAGS) simplex/simplexnoise.cpp -o simplex.o $(INCLUDE)

clean:
	rm $(OBJS) $(TARGET)
