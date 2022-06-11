#include "Shader.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4703)
#endif

#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3dCompiler.h>

typedef struct ShaderData
{
	SHADER_TYPE type;

	union
	{
		struct
		{
			ID3D11VertexShader* vertexShader;
			ID3D11InputLayout* inputLayout;
		};

		ID3D11PixelShader* pixelShader;
	};

	const SCHAR* filePath;
	const char* entrypoint;
	FILETIME lastWrite;

} ShaderData;

static BOOL LoadFile(const SCHAR* pPath, char** pData, DWORD* pSize, FILETIME* pLastWrite)
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
		*pSize = fileSize;
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

static ID3DBlob* CompileShader(SHADER_TYPE type, const char* pCode, uint32_t codeSize, const char* pEntrypoint)
{
	ID3DBlob* shader;
	ID3DBlob* error;

	HRESULT hr = D3DCompile(pCode, codeSize, NULL, NULL, NULL, pEntrypoint,
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

static inline BOOL CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pCode, ID3D11PixelShader** pShader)
{
	HRESULT hr = pDevice->lpVtbl->CreatePixelShader(pDevice, pCode->lpVtbl->GetBufferPointer(pCode), pCode->lpVtbl->GetBufferSize(pCode), NULL, pShader);
	return FAILED(hr) ? FALSE : TRUE;
}

static inline BOOL CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pCode, ID3D11VertexShader** pShader)
{
	HRESULT hr = pDevice->lpVtbl->CreateVertexShader(pDevice, pCode->lpVtbl->GetBufferPointer(pCode), pCode->lpVtbl->GetBufferSize(pCode), NULL, pShader);
	return FAILED(hr) ? FALSE : TRUE;
}

SHADER_RESULT CreateShader(ID3D11Device* pDevice, const SCHAR* pPath, const char* pEntrypoint, SHADER_TYPE type, const D3D11_INPUT_ELEMENT_DESC* pLayout, uint32_t layoutcount, ShaderData** pShader)
{
	SHADER_RESULT result = { 0 };
	FILETIME lastWrite = { 0 };
	DWORD fileSize = { 0 };
	char* codeBuffer = { 0 };

	if (type != SHADER_PIXEL && type != SHADER_VERTEX)
	{
		result = SR_INVALID_TYPE;
		goto error;
	}

	BOOL fileStatus = LoadFile(pPath, &codeBuffer, &fileSize, &lastWrite);

	if (!fileStatus)
	{
		result = SR_FILE_LOAD_ERROR;
		goto error;
	}

	ID3DBlob* shaderByteCode = CompileShader(type, codeBuffer, fileSize, pEntrypoint);

	free(codeBuffer);

	if (shaderByteCode == NULL)
	{
		result = SR_SHADER_SYNTAX_ERROR;
		goto error;
	}

	size_t structureSize = sizeof(ShaderData);
	size_t pathSize = strlen(pPath) * sizeof(SCHAR) + 1;
	size_t entrypointSize = strlen(pEntrypoint) + 1;
	size_t totalSize = structureSize + pathSize + entrypointSize;

	ShaderData* data = (ShaderData*)malloc(totalSize);
	if (data == NULL)
	{
		result = SR_OUT_OF_MEMORY;
		goto error;
	}

	data->filePath = ((SCHAR*)data) + structureSize;
	data->entrypoint = data->filePath + pathSize;

	memcpy((char*)data->filePath, pPath, pathSize);
	memcpy((char*)data->entrypoint, pEntrypoint, entrypointSize);

	data->type = type;
	data->lastWrite = lastWrite;

	switch (type)
	{
	case SHADER_VERTEX:
	{
		HRESULT hr = pDevice->lpVtbl->CreateInputLayout(pDevice, pLayout, (UINT)layoutcount, shaderByteCode->lpVtbl->GetBufferPointer(shaderByteCode), shaderByteCode->lpVtbl->GetBufferSize(shaderByteCode), &data->inputLayout);
		if (FAILED(hr))
		{
			result = SR_INVALID_INPUT_LAYOUT;
			goto error;
		}
		BOOL res = CreateVertexShader(pDevice, shaderByteCode, &data->vertexShader);
		if (!res)
		{
			result = SR_SHADER_LINKING_ERROR;
			goto error;
		}
	}
	break;
	case SHADER_PIXEL:
	{
		BOOL res = CreatePixelShader(pDevice, shaderByteCode, &data->pixelShader);
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
			data->inputLayout->lpVtbl->Release(data->inputLayout);
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

void DeleteShader(ShaderData* pShader)
{
	if (pShader == NULL)
		return;
	switch (pShader->type)
	{
	case SHADER_VERTEX:
	{
		pShader->vertexShader->lpVtbl->Release(pShader->vertexShader);
		pShader->inputLayout->lpVtbl->Release(pShader->inputLayout);
	}
	break;
	case SHADER_PIXEL:
	{
		pShader->pixelShader->lpVtbl->Release(pShader->pixelShader);
	}
	break;
	}
	free(pShader);
}

void BindShader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ShaderData* pShader)
{
	if (pShader == NULL || pShader->type == SHADER_INVALID)
		return;

	if (pDevice)
	{
		FILETIME fileWrite = { 0 };
		BOOL result = LoadFile(pShader->filePath, NULL, NULL, &fileWrite);
		if (result && CompareFileTime(&fileWrite, &pShader->lastWrite) > 0)
		{
			pShader->lastWrite = fileWrite;
			char* codeBuffer = NULL;
			uint32_t fileSize = 0;
			result = LoadFile(pShader->filePath, &codeBuffer, &fileSize, NULL);
			if (result)
			{
				if (fileSize > 0)
				{
					ID3DBlob* shaderByteCode = CompileShader(pShader->type, codeBuffer, fileSize, pShader->entrypoint);

					if (shaderByteCode != NULL)
					{
						switch (pShader->type)
						{
						case SHADER_VERTEX:
						{
							ID3D11VertexShader* vs;
							if (CreateVertexShader(pDevice, shaderByteCode, &vs))
							{
								pShader->vertexShader->lpVtbl->Release(pShader->vertexShader);
								pShader->vertexShader = vs;
							}
						}
						break;
						case SHADER_PIXEL:
						{
							ID3D11PixelShader* ps;
							if (CreatePixelShader(pDevice, shaderByteCode, &ps))
							{
								pShader->pixelShader->lpVtbl->Release(pShader->pixelShader);
								pShader->pixelShader = ps;
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
			pDeviceContext->lpVtbl->IASetInputLayout(pDeviceContext, pShader->inputLayout);
			pDeviceContext->lpVtbl->VSSetShader(pDeviceContext, pShader->vertexShader, 0, 0);
			break;
		case SHADER_PIXEL:
			pDeviceContext->lpVtbl->PSSetShader(pDeviceContext, pShader->pixelShader, 0, 0);
			break;
		}
	}
}

#ifdef _MSC_VER
#pragma warning( pop ) 
#endif