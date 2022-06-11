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
	ID3D11ShaderResourceView* shaderResourceView = nullptr;

	// Allow cast to bool ("invariant") to see if this is a valid texture
	operator bool()
	{
		return (bool)shaderResourceView && width && height;
	}
};

/// <summary>
/// Load a texture from file.
/// </summary>
HRESULT LoadTextureFromFile(
	ID3D11Device* dxDevice,
	const char* fileName,
	Texture* textureOut);

/// <summary>
/// Load a texture from file. A mip map is generated if 
/// dxDeviceContext is not null and valid.
/// </summary>
HRESULT LoadTextureFromFile(
	ID3D11Device* dxDevice,
	ID3D11DeviceContext* dxDeviceContext,
	const char* fileName,
	Texture* textureOut);

HRESULT LoadCubeTextureFromFile(
	ID3D11Device* dxDevice,
	const char** fileNames,
	Texture* textureOut);

#endif