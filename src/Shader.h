#pragma once
#include "stdafx.h"

HRESULT CreateShadersAndInputLayout(
	ID3D11Device* device, 
	ID3D11VertexShader** vertexShader, 
	ID3D11PixelShader** pixelShader, 
	ID3D11InputLayout** inputLayout);

HRESULT CompileShader(
	const char* shaderFile, 
	const char* pEntrypoint, 
	const char* pTarget, 
	D3D10_SHADER_MACRO* pDefines, 
	ID3DBlob** pCompiledShader);