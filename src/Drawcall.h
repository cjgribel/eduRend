
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
#include "vec/Vec.h"
#include "Texture.h"

using namespace linalg;

struct Vertex
{
	Vec3f pos;
	Vec3f normal, tangent, binormal;
	Vec2f texCoord;
};

//
// Phong-esque material
//
struct Material
{
	Material() = default;
	Material(Vec3f Ka, Vec3f Kd, Vec3f Ks)
		: Ka(Ka), Kd(Kd), Ks(Ks) { }

	// Color components with default values
	Vec3f Ka = { 0.2f, 0.2f, 0.2f }; // Ambient color
	Vec3f Kd = { 0.8f, 0.8f, 0.8f }; // Diffuse color
	Vec3f Ks = { 1.0f, 1.0f, 1.0f }; // Specular color
    
	// Material name
	std::string name;

	// File paths to textures
	std::string diffuseTextureFileName;
	std::string normalTextureFileName;
	// + more texture types (extend OBJLoader::LoadMaterials if needed)

	// Device textures
	Texture diffuseTexture;
	// + other texture types
};

const Material material_default = Material();
const Material material_white   = Material(vec3f_zero, Vec3f(1.0, 1.0, 1.0), Vec3f(1.0, 1.0, 1.0));
const Material material_black   = Material(vec3f_zero, Vec3f(0.3, 0.3, 0.3), Vec3f(0.5, 0.5, 0.5));
const Material material_red     = Material(vec3f_zero, Vec3f(1.0, 0.0, 0.0), Vec3f(1.0, 1.0, 1.0));
const Material material_green   = Material(vec3f_zero, Vec3f(0.0, 1.0, 0.0), Vec3f(1.0, 1.0, 1.0));
const Material material_blue    = Material(vec3f_zero, Vec3f(0.0, 0.0, 1.0), Vec3f(1.0, 1.0, 1.0));

typedef std::unordered_map<std::string, Material> MaterialHash;

struct Triangle 
{ 
	unsigned vi[3]; 
};

struct Quad
{ 
	unsigned vi[4];
};

struct Drawcall
{
    std::string groupName;
    int materialIndex = -1;
    std::vector<Triangle> tris;
    std::vector<Quad> quads;
    
	// Make sortable w.r.t. material
    bool operator < (const Drawcall& drawcall) const
    {
        return materialIndex < drawcall.materialIndex;
    }
};

#endif
