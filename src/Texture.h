//
// Texture loaders
// Adapted from:
// https://github-wiki-see.page/m/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
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

/// <summary>
/// Load a texture from file.
/// </summary>
HRESULT LoadTextureFromFile(
	ID3D11Device* dxdevice,
	const char* filename,
	Texture* texture_out);

/// <summary>
/// Load a texture from file. A mip map is generated if 
/// dxdevice_context is not null and valid.
/// </summary>
HRESULT LoadTextureFromFile(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	const char* filename,
	Texture* texture_out);

HRESULT LoadCubeTextureFromFile(
	ID3D11Device* dxdevice,
	const char** filenames,
	Texture* texture_out);

#endif