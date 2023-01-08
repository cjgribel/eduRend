/*!
*   @file shader.h
*	@brief		Wrapper for DirextX 11 shaders
*	@details	Contains creation, deletion and binding code for DirectX 11 shaders with optional hotreloading of the shaders at bind time.
*	@author		Oliver Öhrström
*	@version	1.0
*	@date		2021-12-07
*	@copyright	MIT License.
*/

#ifndef _SHADER_H
#define _SHADER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

	/*
	* Define this if you want to use wide characters.
	*/
#//define SHADER_USE_WIDECHAR 

	/*
	* Forward declarations for used types.
	*/
#ifdef SHADER_USE_WIDECHAR //!< Define to select between wide characters or standard characters
#include <wchar.h>
	typedef wchar_t SCHAR; //!< @private
#else
	typedef char	SCHAR; //!< @private
#endif
	typedef struct ID3D11Device ID3D11Device; //!< @private
	typedef struct ID3D11DeviceContext ID3D11DeviceContext; //!< @private
	typedef struct D3D11_INPUT_ELEMENT_DESC D3D11_INPUT_ELEMENT_DESC; //!< @private

	/**
	 * @brief Opaque data structure containing the shader data.
	 * @see create_shader(ID3D11Device*, const SCHAR*, const char*, SHADER_TYPE, const D3D11_INPUT_ELEMENT_DESC*, uint32_t, shader_data**)
	 * @see delete_shader(shader_data*)
	 * @see bind_shader(ID3D11Device*, ID3D11DeviceContext*, shader_data*)
	*/
	typedef struct shader_data shader_data;

	/**
	 * @brief Defines shader types
	*/
	typedef enum SHADER_TYPE
	{
		SHADER_INVALID = 0, //!< Invalid shader type
		SHADER_VERTEX = 1, //!< Vertex shader type
		SHADER_PIXEL = 2, //!< Pixel shader type
	} SHADER_TYPE;

	/**
	 * @brief Return codes for shader_data creation.
	 * @see create_shader(ID3D11Device*, const SCHAR*, const char*, SHADER_TYPE, const D3D11_INPUT_ELEMENT_DESC*, uint32_t, shader_data**)
	*/
	typedef enum SHADER_RESULT
	{
		SR_OK = 0,
		SR_INVALID_TYPE = -1,
		SR_FILE_LOAD_ERROR = -2,
		SR_SHADER_SYNTAX_ERROR = -3,
		SR_OUT_OF_MEMORY = -4,
		SR_INVALID_INPUT_LAYOUT = -5,
		SR_SHADER_LINKING_ERROR = -6,
	} SHADER_RESULT;

	/**
	 * @brief Create a shader from a text file containing the HLSL code.
	 * @param[in] pDevice Pointer to the active DX11 device.
	 * @param[in] pPath Path the the HLSL file.
	 * @param[in] pEntrypoint Name of the main function within the shader.
	 * @param[in] type Type of shader to build.
	 * @param[in] pLayout Pointer to a valid input layout, this param can be left as N
	 * @param[in] layoutcount Amount of input descriptios in the layout.
	 * @param[out] pShader Pointer to the resulting shader_data.
	 * @return Code describing the result of the creation, returns SR_OK on success.
	*/
	SHADER_RESULT create_shader(ID3D11Device* pDevice, const SCHAR* pPath, const char* pEntrypoint, SHADER_TYPE type, const D3D11_INPUT_ELEMENT_DESC* pLayout, uint32_t layoutcount, shader_data** pShader);

	/**
	 * @brief Deletes a shader created using create_shader.
	 * @see create_shader(ID3D11Device*, const SCHAR*, const char*, SHADER_TYPEm const D3D11_INPUT_ELEMENT_DESC*, uint32_t, shader_data**)
	 * @param[in] pShader Pointer to the shader that should be deleted.
	*/
	void delete_shader(shader_data* pShader);

	/**
	 * @brief Bind a shader to the DX11 pipeline.
	 * @details This function does hot reloading of the shader if a ID3D11DeviceContext is supplied.
	 *
	 * @param[in] pDevice Pointer to the active DX11 device, if this is left to NULL no reload of the shader will be done.
	 * @param[in] pDeviceContext Pointer to the DX11 context that the shaders should be bound using, if NULL the shader binding will be skipped.
	 * @param[in] pShader Pointer to the shader that should be bound.
	*/
	void bind_shader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, shader_data* pShader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SHADER_H