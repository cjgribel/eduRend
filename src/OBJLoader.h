
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

// Make sure loaded normals face in the same direction
// as the triangle's CCW normal
#define MESH_FORCE_CCW
// Sort drawcalls based on material - usually a good idea
#define MESH_SORT_DRAWCALLS

// Accepted image formats
// Note: this is a short list, more formats may be accepted -
// see https://github.com/nothings/stb/blob/master/stb_image.h
#define ALLOWED_TEXTURE_SUFFIXES { "bmp", "jpg", "png", "tga", "gif" }

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

    bool has_normals = false;
    bool has_texcoords = false;

    std::vector<Vertex> vertices;
    std::vector<Drawcall> drawcalls;
    std::vector<Material> materials;
};

#endif
