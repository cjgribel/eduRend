/**
 * @file texture.h
 * @brief Texture loaders
 * @details Adapted from: https://github-wiki-see.page/m/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
*/

#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <utility>
#include <vector>
//#include <wrl/client.h>
#include "stdafx.h"

//using Microsoft::WRL::ComPtr;

/**
 * @brief Represents a texture.
*/
struct Texture
{
	int Width = 0; //!< Width if the Texture in pixels
	int Weight = 0; //!< Height of the Texture in pixels
	ID3D11ShaderResourceView* TextureView = nullptr; //!< Shader Resource View pointing to the GPU texture.

	/**
	 * @brief Allow cast to bool ("invariant") to see if this is a valid texture
	*/
	operator bool() { return (bool)TextureView && Width && Weight; }
};

/**
 * @brief Loads a 2D texture from file.
 * @details Calls LoadTextureFromFile(ID3D11Device*,ID3D11DeviceContext*,const char*,Texture*) for the actual work.
 * @param[in] dxdevice Valid ID3D11Device device.
 * @param[in] filename File path to a valid image.
 * @param[out] texture_out Texture struct to store the resulting texture in.
 * @return HRESULT of the texture creation.
*/
HRESULT LoadTextureFromFile(ID3D11Device* dxdevice,	const char* filename, Texture* texture_out);

/**
 * @brief Loads a 2D texture from file.
 * @param[in] dxdevice Valid ID3D11Device device.
 * @param[in] dxdevice_context If provided the ID3D11DeviceContext will be used to auto generate mip maps for the texture.
 * @param[in] filename File path to a valid image.
 * @param[out] texture_out Texture struct to store the resulting texture in.
 * @return HRESULT of the texture creation.
*/
HRESULT LoadTextureFromFile(ID3D11Device* dxdevice,	ID3D11DeviceContext* dxdevice_context, const char* filename, Texture* texture_out);

/**
 * @brief Loads a 3D texture from 6 individual images.
 * @param[in] dxdevice Valid ID3D11Device device.
 * @param[in] filenames A List of 6 file paths to valid images.
 * @param[out] texture_out Texture struct to store the resulting texture in.
 * @return HRESULT of the texture creation.
*/
HRESULT LoadCubeTextureFromFile(ID3D11Device* dxdevice,	const char** filenames,	Texture* texture_out);

#endif