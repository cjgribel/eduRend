//
//  Texture.h
//
//  Oliver Öhrström
//	CJ Gribel
//

#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <utility>
#include <vector>
//#include <wrl/client.h>
#include "stdafx.h"

//using Microsoft::WRL::ComPtr;

struct Texture
{
	int width = 0;
	int height = 0;
	ID3D11ShaderResourceView* texture_SRV = nullptr;

	// Allow cast to bool ("invariant") to see if this is a valid texture
	operator bool() { return (bool)texture_SRV && width && height; }
};

HRESULT LoadTextureFromFile(
	ID3D11Device* dxdevice,
	const char* filename,
	Texture* texture_out);

HRESULT LoadCubeTextureFromFile(
	ID3D11Device* dxdevice,
	const char** filenames,
	Texture* texture_out);

#endif