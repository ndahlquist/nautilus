#ifndef MESH_DEFINITIONS_H
#define MESH_DEFINITIONS_H

#include <cassert>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>

struct MyTraits : public OpenMesh::DefaultTraits
{
  VertexAttributes(OpenMesh::Attributes::Normal);

  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
  VertexTraits {
    int v_index;
    OpenMesh::Vec2f texture;
  };
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> Mesh;

#endif
