
//
//  OBJ/MTL loader
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include "Drawcall.h"

#define MESH_FORCE_CCW
#define MESH_SORT_DRAWCALLS

// Accepted image formats
#define ALLOWED_TEXTURE_SUFFIXES { "bmp", "jpg", "png", "tiff", "gif" }

//
// OBJ Loader
//
// Parses OBJ/MTL-files and organizes the data in arrays 
// with vertices, drawcalls and materials
//
class OBJLoader
{
    void LoadMaterials(
        std::string dir,
        std::string filename,
        MaterialHash& mtl_hash);
public:

    void Load(
        const std::string& filename,
        bool auto_generate_normals = true,
        bool triangulate = true);

    bool has_normals, has_texcoords;

    std::vector<Vertex> vertices;
    std::vector<Drawcall> drawcalls;
    std::vector<Material> materials;
};

#endif
