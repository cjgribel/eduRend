/*!
*	@brief		Wrapper for DirextX 11 shaders
*	@details	Contains creation, deletion and binding code for DirectX 11 shaders with optional hotreloading of the shaders at bind time.
*	@author		Oliver Öhrström 
*	@version	1.0
*	@date		2021-12-07
*	@copyright	MIT License.
*/

#ifndef _SHADER_H
#define _SHADER_H

#include <D3D11.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
/*
* Define this if you want to use unicode characters instead of ASCII.
*/
#//define SHADER_USE_WIDECHAR 
#ifdef SHADER_USE_WIDECHAR
#include <wchar.h>
	typedef wchar_t SCHAR;
#else
	typedef char	SCHAR;
#endif

	///
	/// Opaque data structure containing the shader data.
	///
	typedef struct shader_data shader_data;

	typedef enum SHADER_TYPE
	{
		SHADER_INVALID = 0,
		SHADER_VERTEX = 1,
		SHADER_PIXEL = 2,
	} SHADER_TYPE;

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

	/// 
	/// Create a shader from a text file containing the HLSL code.
	/// 
	/// @param pDevice Pointer to the active DX11 device.
	/// @param pPath Path the the HLSL file.
	/// @param pEntrypoint Name of the main function within the shader.
	/// @param type Type of shader to build. </param>
	/// @param pLayout Pointer to a valid input layout, this param can be left as NULL for any shader except Vertex.
	/// @param layoutcount Amount of input descriptios in the layout.
	/// @param pShader Pointer to the resulting shader_data.
	/// @return Code describing the result of the creation, returns SR_OK on success.
	/// 
	SHADER_RESULT create_shader(ID3D11Device* pDevice, const SCHAR* pPath, const char* pEntrypoint, SHADER_TYPE type, const D3D11_INPUT_ELEMENT_DESC* pLayout, uint32_t layoutcount, shader_data** pShader);
	
	///
	/// Deletes a shader created using create_shader.
	/// 
	/// @see create_shader()
	/// @param pShader Pointer to the shader that should be deleted.
	/// 
	void delete_shader(shader_data* pShader);

	/// 
	/// Bind a shader to the DX11 pipeline.
	/// This function does hot reloading of the shader if a device is supplied.
	/// 
	/// @param pDevice Pointer to the active DX11 device, if this is left to NULL no reload of the shader will be done.
	/// @param pDeviceContext Pointer to the DX11 context that the shaders should be bound using, if NULL the shader binding will be skipped.
	/// @param pShader Pointer to the shader that should be bound.
	/// 
	void bind_shader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, shader_data* pShader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SHADER_H