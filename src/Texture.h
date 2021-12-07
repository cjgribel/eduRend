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
	const char* filename;
	int width = 0;
	int height = 0;
	ID3D11ShaderResourceView** texture_srv = nullptr;
};

struct CubeTexture
{
	const char* filenames[6];
	int width;
	int height;
    ID3D11ShaderResourceView* texture_srv = nullptr;
};

bool LoadTextureFromFile(
	ID3D11Device* dxdevice,
	const char* filenames,
	ID3D11ShaderResourceView** out_srv,
	int* out_width,
	int* out_height);

bool LoadCubeTextureFromFile(
	ID3D11Device* dxdevice,
	const char** filename,
	ID3D11ShaderResourceView** out_srv,
	int* out_width,
	int* out_height);

#endif