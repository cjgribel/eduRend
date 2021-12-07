
//
//	eduRend 
//	DirectX framework for DA307A Computer Graphics & Modelling
//	Malmö University
//
//	Contributors:
//
//	Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//	Jonas Petersson
//	Alexander Baldwin
//	Oliver Öhrström
//	Hugo Hansen
//

#define VSYNC
#define USECONSOLE

#include "stdafx.h"
#include "shader.h"
#include "Window.h"
#include "ShaderBuffers.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Geometry.h"
#include "Scene.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
IDXGISwapChain*         g_SwapChain				= nullptr;
ID3D11RenderTargetView* g_RenderTargetView		= nullptr;
ID3D11Texture2D*        g_DepthStencil			= nullptr;
ID3D11DepthStencilView* g_DepthStencilView		= nullptr;
ID3D11Device*			g_Device				= nullptr;
ID3D11DeviceContext*	g_DeviceContext			= nullptr;
ID3D11RasterizerState*	g_RasterState			= nullptr;

shader_data*			g_VertexShader			= nullptr;
shader_data*			g_PixelShader			= nullptr;
InputHandler*			g_InputHandler			= nullptr;

ID3D11Buffer*			g_MatrixBuffer			= nullptr;

const int g_InitialWinWidth = 1024;
const int g_InitialWinHeight = 576;
std::unique_ptr<Window> g_Window;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT				Render(float deltaTime);
HRESULT				Update(float deltaTime);
HRESULT				InitDirect3DAndSwapChain(int width, int height);
void				InitRasterizerState();
HRESULT				CreateRenderTargetView();
HRESULT				CreateDepthStencilView(int width, int height);
void				SetViewport(int width, int height);
void				InitShaderBuffers();
void				Release();
void				WinResize();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// load console and redirect some I/O to it
	// note: this has to be done before the win32 window is initialized, otherwise DirectInput dies
#ifdef USECONSOLE
	AllocConsole();
	{
		FILE* fpstdin = stdin, * fpstdout = stdout, * fpstderr = stderr;
		freopen_s(&fpstdin, "conin$", "r", stdin);
		freopen_s(&fpstdout, "conout$", "w", stdout);
		freopen_s(&fpstderr, "conout$", "w", stderr);
	}
#endif
	
	// init the win32 window
	g_Window = std::make_unique<Window>(hInstance, nCmdShow, g_InitialWinWidth, g_InitialWinHeight);

#ifdef USECONSOLE
	printf("Win32-window created...\n");
#ifdef _DEBUG
	printf("Running in DEBUG mode\n");
#else
	printf("Running in RELEASE mode\n");
#endif
#endif

	HRESULT hr = S_OK;

	if(SUCCEEDED(hr = InitDirect3DAndSwapChain(g_InitialWinWidth, g_InitialWinHeight)))
	{
		InitRasterizerState();

		if (SUCCEEDED(hr = CreateRenderTargetView()) &&
			SUCCEEDED(hr = CreateDepthStencilView(g_InitialWinWidth, g_InitialWinHeight)))
		{
			SetViewport(g_InitialWinWidth, g_InitialWinHeight);

			g_DeviceContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );

			const D3D11_INPUT_ELEMENT_DESC inputDesc[5] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			if (FAILED(create_shader(g_Device, "shaders/default_shader.vs", "VS_main", SHADER_VERTEX, &inputDesc[0], 5, &g_VertexShader)) || FAILED(create_shader(g_Device, "shaders/default_shader.ps", "PS_main", SHADER_PIXEL, nullptr, 0, &g_PixelShader)))
			{
				throw std::runtime_error("Failed to create shaders");
			}
			InitShaderBuffers();
			initObjects(g_InitialWinWidth, g_InitialWinHeight, g_Device, g_DeviceContext);
		}
	}

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	g_InputHandler = new InputHandler();
	g_InputHandler->Initialize(hInstance, g_Window->GetHandle(), g_InitialWinWidth, g_InitialWinHeight);

	printf("Entering main loop...\n");
	
	while (g_Window->Update())
	{
		if (g_Window->SizeChanged())
		{
			WinResize();
		}

		__int64 currTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
		const float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;
		g_InputHandler->Update();
		
		Update(dt);
		Render(dt);

		prevTimeStamp = currTimeStamp;
	}

	Release();
#ifdef USECONSOLE
	FreeConsole();
#endif
	return 0;
}

// Resize render targets and swap chains.
// If additional render targets are used (e.g. for shadow mapping),
// they need to be handled here as well.
// Todo: do proper error checking
void WinResize()
{
	const linalg::vec2i size = g_Window->GetSize();

	printf("window resized to %i x %i\n", size.x, size.y);

	if (!g_SwapChain) return;

	g_DeviceContext->OMSetRenderTargets(0, 0, 0);
	// Release all outstanding references to the swap chain's buffers.
	g_RenderTargetView->Release();

	HRESULT hr;
	// Preserve the existing buffer count and format.
	// Automatically choose the width and height to match the client rect for HWNDs.
	hr = g_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	// Perform error handling here!

	// Get buffer and create a render-target-view.
	ID3D11Texture2D* pBuffer;
	hr = g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(void**)&pBuffer);
	// Perform error handling here!

	hr = g_Device->CreateRenderTargetView(pBuffer, NULL, &g_RenderTargetView);
	// Perform error handling here!

	pBuffer->Release();
	SAFE_RELEASE(g_DepthStencil);
	SAFE_RELEASE(g_DepthStencilView);

	CreateDepthStencilView(size.x, size.y);

	g_DeviceContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = (float)size.x;
	vp.Height = (float)size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_DeviceContext->RSSetViewports(1, &vp);

	WindowResize(size.x, size.y);
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDirect3DAndSwapChain(int width, int height)
{
	D3D_DRIVER_TYPE driverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };

	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_Window->GetHandle(); // g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	HRESULT hr = E_FAIL;
	for( UINT driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE(driverTypes) && FAILED(hr); driverTypeIndex++ )
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverTypes[driverTypeIndex],
			nullptr,
			0,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&g_SwapChain,
			&g_Device,
			&initiatedFeatureLevel,
			&g_DeviceContext);
	}
	return hr;
}

void InitRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;

	g_Device->CreateRasterizerState(&rasterizerState, &g_RasterState);
	g_DeviceContext->RSSetState(g_RasterState);
}

void InitShaderBuffers()
{
	HRESULT hr;

	// Matrix buffer
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(MatrixBuffer_t);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;

	ASSERT(hr = g_Device->CreateBuffer(&MatrixBuffer_desc, nullptr, &g_MatrixBuffer));
}

HRESULT CreateRenderTargetView()
{
	HRESULT hr = S_OK;
	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	if(SUCCEEDED(hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer)))
	{
		hr = g_Device->CreateRenderTargetView( pBackBuffer, nullptr, &g_RenderTargetView );
		SAFE_RELEASE(pBackBuffer);
	}

	return hr;
}

HRESULT CreateDepthStencilView(int width, int height)
{
	HRESULT hr = S_OK;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_Device->CreateTexture2D( &descDepth, nullptr, &g_DepthStencil );
	if( FAILED(hr) )
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_Device->CreateDepthStencilView( g_DepthStencil, &descDSV, &g_DepthStencilView );
	
	return hr;
}

void SetViewport(int width, int height)
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_DeviceContext->RSSetViewports( 1, &vp );
}

HRESULT Update(float deltaTime)
{
	updateObjects(deltaTime, g_InputHandler);

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	// Clear back buffer, black color
	static float ClearColor[4] = { 0, 0, 0, 1 };
	g_DeviceContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );
	
	// Clear depth buffer
	g_DeviceContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	
	// Set topology
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
	// Set shaders
	bind_shader(g_Device, g_DeviceContext, g_VertexShader);
	bind_shader(g_Device, g_DeviceContext, g_PixelShader);
	//g_DeviceContext->VSSetShader(g_VertexShader, nullptr, 0);
	g_DeviceContext->HSSetShader(nullptr, nullptr, 0);
	g_DeviceContext->DSSetShader(nullptr, nullptr, 0);
	g_DeviceContext->GSSetShader(nullptr, nullptr, 0);
	//g_DeviceContext->PSSetShader(g_PixelShader, nullptr, 0);
	
	// Set matrix buffers
	g_DeviceContext->VSSetConstantBuffers(0, 1, &g_MatrixBuffer);

	// Time to render our objects
	renderObjects(g_MatrixBuffer, g_DeviceContext);

	// Swap front and back buffer
#ifdef VSYNC
	return g_SwapChain->Present(1, 0);
#else
	return g_SwapChain->Present(0, 0);
#endif
}

void Release()
{
	// deallocate objects
	releaseObjects();

	// free D3D stuff
	SAFE_DELETE(g_InputHandler);

	delete_shader(g_VertexShader);
	delete_shader(g_PixelShader);

	SAFE_RELEASE(g_SwapChain);
	SAFE_RELEASE(g_RenderTargetView);
	SAFE_RELEASE(g_DepthStencil);
	SAFE_RELEASE(g_DepthStencilView);
	SAFE_RELEASE(g_RasterState);

	SAFE_RELEASE(g_DeviceContext);
	SAFE_RELEASE(g_Device);
}