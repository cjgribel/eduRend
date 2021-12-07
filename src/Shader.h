#pragma once
#include <D3D11.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
	// Forward declare DX11 types
	typedef struct ID3D11Device ID3D11Device;
	typedef struct ID3D11DeviceContext ID3D11DeviceContext;

	typedef struct shader_data shader_data;

	typedef enum
	{
		SHADER_INVALID = 0,
		SHADER_VERTEX = 1,
		SHADER_PIXEL = 2,
	} SHADER_TYPE;

	typedef enum
	{
		ERROR_OK = 0,
		ERROR_INVALID_TYPE = -1,
		ERROR_FILE_LOAD_ERROR = -2,
		ERROR_SHADER_SYNTAX_ERROR = -3,
		ERROR_OUT_OF_MEMORY = -4,
		ERROR_INVALID_INPUT_LAYOUT = -5,
		ERROR_SHADER_LINKING_ERROR = -6,
	} ERROR_CODE;

	ERROR_CODE create_shader(ID3D11Device* pDevice, const char* pPath, const char* pEntrypoint, SHADER_TYPE type, const D3D11_INPUT_ELEMENT_DESC* pLayout, uint32_t layoutcount, shader_data** pShader);
	void delete_shader(shader_data* pShader);
	void bind_shader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, shader_data* pShader);

#ifdef __cplusplus
}
#endif // __cplusplus