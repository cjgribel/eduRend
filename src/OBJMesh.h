
//
//  OBJ/MTL loader
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#ifndef OBJMESH_H
#define OBJMESH_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "vec/vec.h"
#include "Drawcall.h"
#include "parseutil.h"

using linalg::vec3f;
using linalg::vec3ui;

#define MESH_FORCE_CCW
#define MESH_SORT_DRAWCALLS
// Image formats that should be supported by DirectXTex
#define ALLOWED_TEXTURE_SUFFIXES { "bmp", "jpg", "png", "tiff", "gif" }

//
// Auxiliary structs for raw file data
//
struct unwelded_triangle_t { int vi[9]; };
struct unwelded_quad_t { int vi[12]; };
struct unwelded_drawcall_t
{
	std::string mtl_name;
	std::string group_name;
	std::vector<unwelded_triangle_t> tris;
	std::vector<unwelded_quad_t> quads;
	int v_ofs = 0;
};

//
// Creates normals to a set of vertices by averaging the geometric normals of the faces they belong to
//
// If a model lacks normals, this function can be used to create them. Works best for relatively smooth models.
//
static void compute_normals(const std::vector<vec3f> &v, std::vector<vec3f> &vn, std::vector<unwelded_drawcall_t> &drawcalls)
{
    std::vector<vec3f> *v_bin = new std::vector<vec3f>[v.size()];
    
    // bin normals from all faces to vertex bins
    for (unwelded_drawcall_t& dc : drawcalls)
        for(unwelded_triangle_t& tri : dc.tris)
        {
            int a = tri.vi[0], b = tri.vi[1], c = tri.vi[2];
            vec3f v0 = v[a], v1 = v[b], v2 = v[c];
            vec3f n = linalg::normalize((v1-v0)%(v2-v0));
            
            v_bin[a].push_back(n);
            v_bin[b].push_back(n);
            v_bin[c].push_back(n);
            
            memcpy(tri.vi+3, tri.vi, 3*sizeof(int));
        }

    // average binned normals and add to array
    for (unsigned i = 0; i < v.size(); i++)
    {
        vec3f n = vec3f(0,0,0);
        for (unsigned j = 0; j < v_bin[i].size(); j++)
        {
            n += v_bin[i][j];
        }
        n = linalg::normalize(n);
        
        vn.push_back(n);
    }

    delete[] v_bin;
}


//
// OBJ mesh
//
// Parses OBJ/MTL-files and organizes the data in arrays with vertices, drawcalls and materials
//
class objmesh_t
{
public:
    bool has_normals, has_texcoords;
    
    std::vector<vertex_t> vertices;
    std::vector<drawcall_t> drawcalls;
    std::vector<material_t> materials;
    
    void load_mtl(
        std::string dir,
        std::string filename,
        mtl_hash_t &mtl_hash);
    
    void load_obj(
        const std::string& filename,
        bool auto_generate_normals = true,
        bool triangulate = true);
};

#endif
