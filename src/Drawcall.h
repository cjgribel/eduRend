
//
//  Drawcall.h
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

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

struct Vertex
{
	vec3f Pos;
	vec3f Normal, Tangent, Binormal;
	vec2f TexCoord;
};

//
// Phong-esque material
//
struct Material
{
	// Phong color components (ambient, diffuse & specular),
	// with default values
    vec3f Ka = {0,0.5,0}, Kd = {0,0.5,0}, Ks = {1,1,1};
    
	std::string name;
	std::string Kd_texture_filename;
	std::string normal_texture_filename;

	// Device textures
	Texture diffuse_texture;
	// + other texture types
};

static Material DefaultMaterial = Material();

typedef std::unordered_map<std::string, Material> MaterialHash;

struct triangle 
{ 
	unsigned vi[3]; 
};

struct quad_t_
{ 
	unsigned vi[4];
};

struct Drawcall
{
    std::string group_name;
    int mtl_index = -1;
    std::vector<triangle> tris;
    std::vector<quad_t_> quads;
    
	// make sortable w.r.t. material
    bool operator < (const Drawcall& dc) const
    {
        return mtl_index < dc.mtl_index;
    }
};

#endif
