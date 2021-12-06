
//
//  Geometry.h
//
//  Carl Johan Gribel 2016, cjgribel@gmail.com
//

#pragma once
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "stdafx.h"
#include <vector>
#include "vec\vec.h"
#include "vec\mat.h"
#include "ShaderBuffers.h"
#include "Drawcall.h"
#include "OBJMesh.h"

using namespace linalg;

class Geometry_t
{
protected:
	// Pointers to the current device and device context
	ID3D11Device* const			dxdevice;
	ID3D11DeviceContext* const	dxdevice_context;

	// Pointers to the class' vertex & index arrays
	ID3D11Buffer* vertex_buffer = nullptr;
	ID3D11Buffer* index_buffer = nullptr;

public:

	Geometry_t(
		ID3D11Device* dxdevice, 
		ID3D11DeviceContext* dxdevice_context) 
		:	dxdevice(dxdevice),
			dxdevice_context(dxdevice_context)
	{ }

	//
	// Map and update the matrix buffer
	//
	virtual void MapMatrixBuffers(
		ID3D11Buffer* matrix_buffer,
		mat4f ModelToWorldMatrix,
		mat4f WorldToViewMatrix,
		mat4f ProjectionMatrix);

	//
	// Abstract render method: must be implemented by derived classes
	//
	virtual void render() const = 0;

	//
	// Destructor
	//
	virtual ~Geometry_t()
	{ 
		SAFE_RELEASE(vertex_buffer);
		SAFE_RELEASE(index_buffer);
	}
};

class Quad_t : public Geometry_t
{
	unsigned nbr_indices = 0;

public:

	Quad_t(
		ID3D11Device* dx3ddevice,
		ID3D11DeviceContext* dx3ddevice_context);

	virtual void render() const;

	~Quad_t() { }
};

class OBJModel_t : public Geometry_t
{
	// index ranges, representing drawcalls, within an index array
	struct index_range_t
	{
		size_t start;
		size_t size;
		unsigned ofs;
		int mtl_index;
	};

	std::vector<index_range_t> index_ranges;
	std::vector<material_t> materials;

	void append_materials(const std::vector<material_t>& mtl_vec)
	{
		materials.insert(materials.end(), mtl_vec.begin(), mtl_vec.end());
	}

public:

	OBJModel_t(
		const std::string& objfile,
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context);

	virtual void render() const;

	~OBJModel_t() { }
};

#endif