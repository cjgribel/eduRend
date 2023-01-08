/**
 * @file objloader.h
 * @brief OBJ/MTL loader
 * @author Carl Johan Gribel 2016-2021, cjgribel@gmail.com
*/

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include "Drawcall.h"

//! Make sure loaded normals face in the same direction as the triangle's CCW normal
#define MESH_FORCE_CCW

//! Sort drawcalls based on material - usually a good idea
#define MESH_SORT_DRAWCALLS

/** 
 * @brief Accepted image formats
 * @note This is a short list, more formats may be accepted -
 * @see https://github.com/nothings/stb/blob/master/stb_image.h
*/
#define ALLOWED_TEXTURE_SUFFIXES { "bmp", "jpg", "png", "tga", "gif" }

/**
 * @brief OBJ Loader.
 * @details Parses OBJ/MTL-files and organizes the data in arrays with Vertices, Drawcalls and materials.
*/
class OBJLoader
{
    void LoadMaterials(std::string directory, std::string filename, MaterialHash& material_hash);
public:
    /**
     * @brief Loads a .obj file and any linked .mtl file.
     * @param filename Path to the file.
     * @param auto_generate_normals Should normals be automatically generated if they are not contained in the file.
     * @param triangulate Should quads be triangulated.
    */
    void Load(const std::string& filename, bool auto_generate_normals = true, bool triangulate = true);

    bool HasNormals = false; //!< Does the model contain normals.
    bool HasTexcoords = false; //!< Does the model contain uv-coordinates

    std::vector<Vertex> Vertices; //!< Vector of Vertex data
    std::vector<Drawcall> Drawcalls; //!< Vector of Drawcall data
    std::vector<Material> Materials; //!< Vector of Material data
};

#endif
