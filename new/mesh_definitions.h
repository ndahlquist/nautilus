#ifndef MESH_DEFINITIONS_H
#define MESH_DEFINITIONS_H

#include <cassert>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>
// #include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>

struct MyTraits : public OpenMesh::DefaultTraits
{
  VertexAttributes(OpenMesh::Attributes::Normal);

  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
  VertexTraits {
    int v_index;
    //OpenMesh::Vec2f texture;
  };
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> Mesh;
typedef OpenMesh::Subdivider::Uniform::LoopT< Mesh > Loop;
//typedef OpenMesh::Subdivider::Uniform::CompositeT< Mesh > Comp;
typedef OpenMesh::Subdivider::Uniform::Sqrt3T< Mesh > Sqrt3T;

#endif
