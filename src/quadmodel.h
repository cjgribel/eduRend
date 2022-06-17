/**
 * @file OBJModel.h
 * @brief Contains the QuadModel
 * @author Carl Johan Gribel 2016, cjgribel@gmail.com
*/

#pragma once
#include "Model.h"

/**
 * @brief Model reprsenting a Quad.
*/
class QuadModel : public Model
{
	unsigned nbr_indices = 0;

public:
	QuadModel(
		ID3D11Device* dx3ddevice,
		ID3D11DeviceContext* dx3ddevice_context);

	virtual void Render() const;

	~QuadModel() { }
};