/**
 * @file drawcall.h
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
	vec3f Position; //!< 3D coordinate of the vertex
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
	vec3f AmbientColour		= { 0.0f, 0.5f, 0.0f }; //!< Ambient colour component
	vec3f DiffuseColour		= { 0.0f, 0.5f, 0.0f }; //!< Diffuse colour component
	vec3f SpecularColour	= { 1.0f, 1.0f, 1.0f }; //!< Specular colour component
    
	std::string Name; //!< Name of the Material

	// File paths to textures
	std::string DiffuseTextureFilename; //!< Diffuse texture path
	std::string NormalTextureFilename; //!< Normal texture path

	// + more texture types (extend OBJLoader::LoadMaterials if needed)

	// Device textures
	Texture DiffuseTexture; //!< Diffuse Texture
	// + other texture types
};

/**
 * @brief Default material
*/
static Material DefaultMaterial = Material();

/**
 * @brief Map between Material using name as the key.
*/
typedef std::unordered_map<std::string, Material> MaterialHash;

/**
 * @brief Indices representing a triangle
*/
struct Triangle 
{ 
	unsigned VertexIndices[3]; //!< Indices of the triangle
};

/**
 * @brief Indices representing a quad
*/
struct Quad
{ 
	unsigned VertexIndices[4]; //!< Indices of the quad
};

/**
 * @brief Contains the data specifying a drawcall
*/
struct Drawcall
{
    std::string GroupName; //!< Name of the drawcall group
    int MaterialIndex = -1; //!< Index of the material used in the drawcall
    std::vector<Triangle> Triangles; //!< List of the Triangles in the drawcall
    std::vector<Quad> Quads; //!< List of the Quads in the drawcall
    
    /**
     * @brief Used for sorting Drawcalls based on material
    */
    bool operator < (const Drawcall& other) const
    {
        return MaterialIndex < other.MaterialIndex;
    }
};

#endif
