
//
//  Model.h
//
//  Carl Johan Gribel 2016, cjgribel@gmail.com
//

#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "stdafx.h"
#include <vector>
#include "vec\Vec.h"
#include "vec\Mat.h"
#include "Drawcall.h"
#include "OBJLoader.h"
#include "Texture.h"

using namespace linalg;

class Model
{
protected:

	// Pointers to the current device and device context
	ID3D11Device* const			dxDevice;
	ID3D11DeviceContext* const	dxDeviceContext;

	// Pointers to the class' vertex & index arrays
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

public:

	Model(ID3D11Device* dxDevice, ID3D11DeviceContext* dxDeviceContext) 
		: dxDevice(dxDevice),dxDeviceContext(dxDeviceContext) { }

	// Abstract render method: must be implemented by derived classes
	virtual void Render() const = 0;

	// Destructor
	virtual ~Model()
	{ 
		SAFE_RELEASE(vertexBuffer);
		SAFE_RELEASE(indexBuffer);
	}
};

class QuadModel : public Model
{
private:

	unsigned numberOfIndices = 0;

public:

	QuadModel(ID3D11Device* dx3dDevice, ID3D11DeviceContext* dx3dDeviceContext);

	virtual void Render() const;

	~QuadModel() { }
};

class OBJModel : public Model
{
private:

	// Index ranges, representing drawcalls, within an index array
	struct IndexRange
	{
		unsigned int start;
		unsigned int size;
		unsigned ofs;
		int materialIndex;
	};

	std::vector<IndexRange> indexRanges;
	std::vector<Material> materials;

	void AppendMaterials(const std::vector<Material>& newMaterials)
	{
		materials.insert(materials.end(), newMaterials.begin(), newMaterials.end());
	}

public:

	OBJModel(
		const std::string& objFile,
		ID3D11Device* dxDevice,
		ID3D11DeviceContext* dxDeviceContext);

	virtual void Render() const;

	~OBJModel();
};

#endif