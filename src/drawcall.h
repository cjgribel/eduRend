/**
 * @file Drawcall.h
 * @brief Contains Vertex, Material, and Drawcall definitions.
 * @author Carl Johan Gribel 2016-2021, cjgribel@gmail.com
*/

#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "stdafx.h"
#include "vec/vec.h"

#include "Texture.h"

using namespace linalg;

/**
 * @brief Structure defining a vertex
*/
struct Vertex
{
	vec3f Pos; //!< 3D coordinate of the vertex
	vec3f Normal; //!< Normal of the vertex
	vec3f Tangent; //!< Tangent of the vertex
	vec3f Binormal; //!< Binormal of the vertex
	vec2f TexCoord; //!< 2D texture coordiante of the vertex
};

/**
 * @brief Phong-esque material
*/
struct Material
{
	vec3f Ka = { 0,0.5,0 }; //!< Ambient colour component
	vec3f Kd = { 0,0.5,0 }; //!< Diffuse colour component
	vec3f Ks = { 1,1,1 }; //!< Specular colour component
    
	std::string name; //!< Name of the Material

	// File paths to textures
	std::string Kd_texture_filename; //!< Diffuse texture path
	std::string normal_texture_filename; //!< Normal texture path

	// + more texture types (extend OBJLoader::LoadMaterials if needed)

	// Device textures
	Texture diffuse_texture; //!< Diffuse Texture
	// + other texture types
};

/**
 * @brief Default material
*/
static Material DefaultMaterial = Material();

typedef std::unordered_map<std::string, Material> MaterialHash;

/**
 * @brief Indices representing a triangle
*/
struct Triangle 
{ 
	unsigned vi[3]; //!< Indices of the triangle
};

/**
 * @brief Indices representing a quad
*/
struct Quad
{ 
	unsigned vi[4]; //!< Indices of the quad
};

/**
 * @brief Contains the data specifying a drawcall
*/
struct Drawcall
{
    std::string group_name; //!< Name of the drawcall group
    int mtl_index = -1; //!< Index of the material used in the drawcall
    std::vector<Triangle> tris; //!< List of the Triangles in the drawcall
    std::vector<Quad> quads; //!< List of the Quads in the drawcall
    
	// Make sortable w.r.t. material
    /**
     * @brief Used for sorting drawcalls based on material
    */
    bool operator < (const Drawcall& dc) const
    {
        return mtl_index < dc.mtl_index;
    }
};

#endif
