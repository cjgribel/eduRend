#include "Shader.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4703)
#pragma warning( disable : 4701)
#pragma warning( disable : 5105)
#endif

#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>

typedef struct shader_data
{
	SHADER_TYPE type;
	union
	{
		struct
		{
			ID3D11VertexShader* vetex_shader;
			ID3D11InputLayout* input_layout;
		};
		ID3D11PixelShader* pixel_shader;
	};
	const SCHAR* file_path;
	const char* entrypoint;
	FILETIME last_write;
} shader_data;

static BOOL load_file(const SCHAR* pPath, char** pData, uint32_t* pSize, FILETIME* pLastWrite)
{
	BOOL result = { 0 };
	FILETIME lastWrite = { 0 };
	DWORD fileSize = { 0 };
	HANDLE file = INVALID_HANDLE_VALUE;
	char* buffer = NULL;

#ifdef SHADER_USE_WIDECHAR
	file = CreateFileW(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
	file = CreateFileA(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
	if (file == INVALID_HANDLE_VALUE) goto error;

	if (pLastWrite)
	{
		result = GetFileTime(file, NULL, NULL, &lastWrite);
		if (result == FALSE) goto error;
		*pLastWrite = lastWrite;
	}
	if (pData && pSize)
	{
		fileSize = GetFileSize(file, NULL);
		*pSize = (uint32_t)fileSize;
		buffer = (char*)malloc(fileSize);
		if (buffer == NULL) goto error;

		result = ReadFile(file, buffer, fileSize, NULL, NULL);
		if (!result) goto error;
		*pData = buffer;
	}
	CloseHandle(file);
	return TRUE;

error:
	if (buffer)
		free(buffer);
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	return FALSE;
}

static ID3DBlob* compile_shader(SHADER_TYPE type, const char* pCode, uint32_t codeSize, const char* pEntrypoint)
{
	ID3DBlob* shader;
	ID3DBlob* error;

	D3DCompile(pCode, codeSize, NULL, NULL, NULL, pEntrypoint,
		type == SHADER_VERTEX ? "vs_5_0" : "ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS,
		0,
		&shader,
		&error
	);
	if (error)
	{
		printf((char*)error->lpVtbl->GetBufferPointer(error));
		error->lpVtbl->Release(error);
		return NULL;
	}
	return shader;
}

static inline BOOL create_pixel_Shader(ID3D11Device* pDevice, ID3DBlob* pCode, ID3D11PixelShader** pShader)
{
	HRESULT hr = pDevice->lpVtbl->CreatePixelShader(pDevice, pCode->lpVtbl->GetBufferPointer(pCode), pCode->lpVtbl->GetBufferSize(pCode), NULL, pShader);
	return FAILED(hr) ? FALSE : TRUE;
}

static inline BOOL create_vertex_Shader(ID3D11Device* pDevice, ID3DBlob* pCode, ID3D11VertexShader** pShader)
{
	HRESULT hr = pDevice->lpVtbl->CreateVertexShader(pDevice, pCode->lpVtbl->GetBufferPointer(pCode), pCode->lpVtbl->GetBufferSize(pCode), NULL, pShader);
	return FAILED(hr) ? FALSE : TRUE;
}

SHADER_RESULT create_shader(ID3D11Device* pDevice, const SCHAR* pPath, const char* pEntrypoint, SHADER_TYPE type, const D3D11_INPUT_ELEMENT_DESC* pLayout, uint32_t layoutcount, shader_data** pShader)
{
	SHADER_RESULT result = { 0 };
	FILETIME lastWrite = { 0 };
	uint32_t fileSize = { 0 };
	char* codeBuffer = { 0 };

	if (type != SHADER_PIXEL && type != SHADER_VERTEX)
	{
		result = SR_INVALID_TYPE;
		goto error;
	}

	BOOL fileStatus = load_file(pPath, &codeBuffer, &fileSize, &lastWrite);

	if (!fileStatus)
	{
		result = SR_FILE_LOAD_ERROR;
		goto error;
	}

	ID3DBlob* shaderByteCode = {0};
	shaderByteCode = compile_shader(type, codeBuffer, fileSize, pEntrypoint);

	free(codeBuffer);

	if (shaderByteCode == NULL)
	{
		result = SR_SHADER_SYNTAX_ERROR;
		goto error;
	}

	size_t structureSize = sizeof(shader_data);
	size_t pathSize = strlen(pPath) * sizeof(SCHAR) + 1;
	size_t entrypointSize = strlen(pEntrypoint) + 1;
	size_t totalSize = structureSize + pathSize + entrypointSize;

	shader_data* data = {0};
	data = (shader_data*)malloc(totalSize);
	if (data == NULL)
	{
		result = SR_OUT_OF_MEMORY;
		goto error;
	}

	data->file_path = ((SCHAR*)data) + structureSize;
	data->entrypoint = data->file_path + pathSize;

	memcpy((char*)data->file_path, pPath, pathSize);
	memcpy((char*)data->entrypoint, pEntrypoint, entrypointSize);

	data->type = type;
	data->last_write = lastWrite;

	switch (type)
	{
	case SHADER_VERTEX:
	{
		HRESULT hr = pDevice->lpVtbl->CreateInputLayout(pDevice, pLayout, (UINT)layoutcount, shaderByteCode->lpVtbl->GetBufferPointer(shaderByteCode), shaderByteCode->lpVtbl->GetBufferSize(shaderByteCode), &data->input_layout);
		if (FAILED(hr))
		{
			result = SR_INVALID_INPUT_LAYOUT;
			goto error;
		}
		BOOL res = create_vertex_Shader(pDevice, shaderByteCode, &data->vetex_shader);
		if (!res)
		{
			result = SR_SHADER_LINKING_ERROR;
			goto error;
		}
	}
	break;
	case SHADER_PIXEL:
	{
		BOOL res = create_pixel_Shader(pDevice, shaderByteCode, &data->pixel_shader);
		if (!res)
		{
			result = SR_SHADER_LINKING_ERROR;
			goto error;
		}
	}
	}
	shaderByteCode->lpVtbl->Release(shaderByteCode);
	*pShader = data;
	return result;

error:
	switch (result)
	{
		case SR_SHADER_LINKING_ERROR:
		{
			if (type == SHADER_VERTEX)
				data->input_layout->lpVtbl->Release(data->input_layout);
		}
		case SR_INVALID_INPUT_LAYOUT:
		{
			free(data);
		}
		case SR_OUT_OF_MEMORY:
		{
			shaderByteCode->lpVtbl->Release(shaderByteCode);
		}
	}
	return result;
}

void delete_shader(shader_data* pShader)
{
	if (pShader == NULL)
		return;
	switch (pShader->type)
	{
	case SHADER_VERTEX:
	{
		pShader->vetex_shader->lpVtbl->Release(pShader->vetex_shader);
		pShader->input_layout->lpVtbl->Release(pShader->input_layout);
	}
	break;
	case SHADER_PIXEL:
	{
		pShader->pixel_shader->lpVtbl->Release(pShader->pixel_shader);
	}
	break;
	}
	free(pShader);
}

void bind_shader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, shader_data* pShader)
{
	if (pShader == NULL || pShader->type == SHADER_INVALID)
		return;

	if (pDevice)
	{
		FILETIME file_write = { 0 };
		BOOL result = load_file(pShader->file_path, NULL, NULL, &file_write);
		if (result && CompareFileTime(&file_write, &pShader->last_write) > 0)
		{
			pShader->last_write = file_write;
			char* codeBuffer = NULL;
			uint32_t fileSize = 0;
			result = load_file(pShader->file_path, &codeBuffer, &fileSize, NULL);
			if (result)
			{
				if (fileSize > 0)
				{
					ID3DBlob* shaderByteCode = compile_shader(pShader->type, codeBuffer, fileSize, pShader->entrypoint);

					if (shaderByteCode != NULL)
					{
						switch (pShader->type)
						{
						case SHADER_VERTEX:
						{
							ID3D11VertexShader* vs;
							if (create_vertex_Shader(pDevice, shaderByteCode, &vs))
							{
								pShader->vetex_shader->lpVtbl->Release(pShader->vetex_shader);
								pShader->vetex_shader = vs;
							}
						}
						break;
						case SHADER_PIXEL:
						{
							ID3D11PixelShader* ps;
							if (create_pixel_Shader(pDevice, shaderByteCode, &ps))
							{
								pShader->pixel_shader->lpVtbl->Release(pShader->pixel_shader);
								pShader->pixel_shader = ps;
							}
						}
						break;
						}

						shaderByteCode->lpVtbl->Release(shaderByteCode);
					}
				}
				free(codeBuffer);
			}
		}
	}

	if (pDeviceContext)
	{
		switch (pShader->type)
		{
		case SHADER_VERTEX:
			pDeviceContext->lpVtbl->IASetInputLayout(pDeviceContext, pShader->input_layout);
			pDeviceContext->lpVtbl->VSSetShader(pDeviceContext, pShader->vetex_shader, 0, 0);
			break;
		case SHADER_PIXEL:
			pDeviceContext->lpVtbl->PSSetShader(pDeviceContext, pShader->pixel_shader, 0, 0);
			break;
		}
	}
}

#ifdef _MSC_VER
#pragma warning( pop ) 
#endif