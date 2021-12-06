#include "Shader.h"

HRESULT CreateShadersAndInputLayout(
	ID3D11Device* device, 
	ID3D11VertexShader** vertexShader, 
	ID3D11PixelShader** pixelShader,
	ID3D11InputLayout** inputLayout)
{
	HRESULT hr = S_OK;

	printf("\nCompiling vertex shader...\n");
	ID3DBlob* pVertexShader = nullptr;
	if (SUCCEEDED(hr = CompileShader("shaders/DrawTri.vs", "VS_main", "vs_5_0", nullptr, &pVertexShader)))
	{
		if (SUCCEEDED(hr = device->CreateVertexShader(
			pVertexShader->GetBufferPointer(),
			pVertexShader->GetBufferSize(),
			nullptr,
			vertexShader)))
		{
			D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			hr = device->CreateInputLayout(
				inputDesc,
				ARRAYSIZE(inputDesc),
				pVertexShader->GetBufferPointer(),
				pVertexShader->GetBufferSize(),
				inputLayout);
		}

		SAFE_RELEASE(pVertexShader);
	}
	else
	{
		MessageBoxA(nullptr, "Failed to create vertex shader (check Output window for more info)", 0, 0);
	}

	printf("\nCompiling pixel shader...\n\n");
	ID3DBlob* pPixelShader = nullptr;
	if (SUCCEEDED(hr = CompileShader("shaders/DrawTri.ps", "PS_main", "ps_5_0", nullptr, &pPixelShader)))
	{
		hr = device->CreatePixelShader(
			pPixelShader->GetBufferPointer(),
			pPixelShader->GetBufferSize(),
			nullptr,
			pixelShader);

		SAFE_RELEASE(pPixelShader);
	}
	else
	{
		MessageBoxA(nullptr, "Failed to create pixel shader (check Output window for more info)", 0, 0);
	}

	return hr;
}

HRESULT CompileShader(
	const char* shaderFile, 
	const char* pEntrypoint, 
	const char* pTarget, 
	D3D10_SHADER_MACRO* pDefines, 
	ID3DBlob** pCompiledShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS |
		D3DCOMPILE_IEEE_STRICTNESS;

	std::string shader_code;
	std::ifstream in(shaderFile, std::ios::in | std::ios::binary);
	if (in)
	{
		in.seekg(0, std::ios::end);
		shader_code.resize((UINT)in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&shader_code[0], shader_code.size());
		in.close();
	}

	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr = D3DCompile(
		shader_code.data(),
		shader_code.size(),
		nullptr,
		pDefines,
		nullptr,
		pEntrypoint,
		pTarget,
		dwShaderFlags,
		0,
		pCompiledShader,
		&pErrorBlob);

	if (pErrorBlob)
	{
		// output error message
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
#ifdef USECONSOLE
		printf("%s\n", (char*)pErrorBlob->GetBufferPointer());
#endif
		SAFE_RELEASE(pErrorBlob);
	}

	return hr;
}