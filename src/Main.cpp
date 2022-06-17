
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
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Scene.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static IDXGISwapChain*			g_SwapChain				= nullptr;
static ID3D11RenderTargetView*	g_RenderTargetView		= nullptr;
static ID3D11Texture2D*			g_DepthStencil			= nullptr;
static ID3D11DepthStencilView*	g_DepthStencilView		= nullptr;
static ID3D11Device*			g_Device				= nullptr;
static ID3D11DeviceContext*		g_DeviceContext			= nullptr;
static ID3D11RasterizerState*	g_RasterState			= nullptr;

static shader_data*				g_VertexShader			= nullptr;
static shader_data*				g_PixelShader			= nullptr;

#ifdef _DEBUG
static ID3D11Debug*				g_DebugController		= nullptr;
#endif // _DEBUG

static const int g_InitialWinWidth = 1024;
static const int g_InitialWinHeight = 576;

static InputHandler			g_InputHandler;
static Window				g_Window;
static std::unique_ptr<Scene> scene;

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
//void				InitShaderBuffers();
void				Release();
void				WinResize();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
	// Load console and redirect some I/O to it
	// Note: this has to be done before the win32 window is initialized, otherwise DirectInput dies
#ifdef USECONSOLE
	AllocConsole();
	{
		FILE* fpstdin = stdin;
		FILE* fpstdout = stdout;
		FILE* fpstderr = stderr;
		freopen_s(&fpstdin, "conin$", "r", stdin);
		freopen_s(&fpstdout, "conout$", "w", stdout);
		freopen_s(&fpstderr, "conout$", "w", stderr);
	}
#endif
	
	// Init the win32 window
	g_Window.Init(g_InitialWinWidth, g_InitialWinHeight);

	g_InputHandler.Initialize(hInstance, g_Window.GetHandle(), g_InitialWinWidth, g_InitialWinHeight);

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

			if (FAILED(create_shader(g_Device,  "shaders/vertex_shader.hlsl", "VS_main", SHADER_VERTEX, &inputDesc[0], 5, &g_VertexShader)) || 
				FAILED(create_shader(g_Device, "shaders/pixel_shader.hlsl", "PS_main", SHADER_PIXEL, nullptr, 0, &g_PixelShader)))
			{
				__debugbreak();
			}

			scene = std::make_unique<OurTestScene>(
				g_Device,
				g_DeviceContext,
				g_InitialWinWidth,
				g_InitialWinHeight);
			scene->Init();
		}
	}

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	

	printf("Entering main loop...\n");
	
	while (g_Window.Update())
	{
		if (g_Window.SizeChanged())
		{
			WinResize();
		}

		__int64 currTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
		const float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;
		g_InputHandler.Update();
		
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
void WinResize()
{
	const linalg::vec2i size = g_Window.GetSize();

	printf("window resized to %i x %i\n", size.x, size.y);

	if (!g_SwapChain) return;

	g_DeviceContext->OMSetRenderTargets(0, 0, 0);
	// Release all outstanding references to the swap chain's buffers.
	g_RenderTargetView->Release();

	HRESULT hr;
	// Preserve the existing buffer count and format.
	// Automatically choose the width and height to match the client rect for HWNDs.
	ASSERT(hr = g_SwapChain->ResizeBuffers(
		0,
		0,
		0,
		DXGI_FORMAT_UNKNOWN,
		0));

	// Get buffer and create a render-target-view.
	ID3D11Texture2D* pBuffer = nullptr;
	ASSERT(hr = g_SwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&pBuffer));

	ASSERT(hr = g_Device->CreateRenderTargetView(
		pBuffer, 
		NULL, 
		&g_RenderTargetView));
	SETNAME(g_RenderTargetView, "RenderTargetView");

	pBuffer->Release();
	SAFE_RELEASE(g_DepthStencil);
	SAFE_RELEASE(g_DepthStencilView);

	CreateDepthStencilView(size.x, size.y);
	SETNAME(g_RenderTargetView, "RenderTargetView");
	g_DeviceContext->OMSetRenderTargets(
		1, 
		&g_RenderTargetView, 
		g_DepthStencilView);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = (float)size.x;
	vp.Height = (float)size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_DeviceContext->RSSetViewports(1, &vp);

	scene->WindowResize(size.x, size.y);
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDirect3DAndSwapChain(int width, int height)
{
	D3D_DRIVER_TYPE driverTypes[] = 
	{ 
		D3D_DRIVER_TYPE_HARDWARE, 
		D3D_DRIVER_TYPE_WARP, 
		D3D_DRIVER_TYPE_REFERENCE 
	};

	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_Window.GetHandle(); // g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL initiatedFeatureLevel;
	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr = E_FAIL;
	for (UINT driverTypeIndex = 0; 
		driverTypeIndex < ARRAYSIZE(driverTypes) && FAILED(hr); 
		driverTypeIndex++)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverTypes[driverTypeIndex],
			nullptr,
			flags,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&g_SwapChain,
			&g_Device,
			&initiatedFeatureLevel,
			&g_DeviceContext);
	}
#ifdef _DEBUG
	g_Device->QueryInterface(&g_DebugController);
	//g_DebugController->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	SETNAME(g_SwapChain, "Swapchain");
	SETNAME(g_Device, "Device");
	SETNAME(g_DeviceContext, "Context");
#endif // _DEBUG
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
	SETNAME(g_RasterState, "RasterizerState");
	g_DeviceContext->RSSetState(g_RasterState);
}

HRESULT CreateRenderTargetView()
{
	HRESULT hr = S_OK;
	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	if(SUCCEEDED(hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer)))
	{
		hr = g_Device->CreateRenderTargetView( pBackBuffer, nullptr, &g_RenderTargetView );
		SETNAME(g_RenderTargetView, "RenderTargetView");
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
	SETNAME(g_DepthStencil, "DepthStencil");
	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_Device->CreateDepthStencilView( g_DepthStencil, &descDSV, &g_DepthStencilView );
	SETNAME(g_DepthStencilView, "DepthStencilView");
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
	scene->Update(deltaTime, &g_InputHandler);

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	// Get rid of unreferenced warning
	deltaTime = deltaTime;

	// Clear color in RGBA
	static float ClearColor[4] = { 0, 0, 0, 1 };
	// Clear back buffer
	g_DeviceContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );
	
	// Clear depth and stencil buffer
	g_DeviceContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	
	// Set topology
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
	// Bind shaders
	bind_shader(g_Device, g_DeviceContext, g_VertexShader);
	bind_shader(g_Device, g_DeviceContext, g_PixelShader);
	//g_DeviceContext->VSSetShader(g_VertexShader, nullptr, 0);
	g_DeviceContext->HSSetShader(nullptr, nullptr, 0);
	g_DeviceContext->DSSetShader(nullptr, nullptr, 0);
	g_DeviceContext->GSSetShader(nullptr, nullptr, 0);
	//g_DeviceContext->PSSetShader(g_PixelShader, nullptr, 0);
	
	// Time for the current scene to render
	scene->Render();

	// Swap front and back buffer
#ifdef VSYNC
	// Swapping synchronized with monitor
	return g_SwapChain->Present(1, 0);
#else
	// Swapping not synchronized with monitor
	return g_SwapChain->Present(0, 0);
#endif
}

void Release()
{
	SAFE_RELEASE(scene);

	delete_shader(g_VertexShader);
	delete_shader(g_PixelShader);

	SAFE_RELEASE(g_SwapChain);
	SAFE_RELEASE(g_RenderTargetView);
	SAFE_RELEASE(g_DepthStencil);
	SAFE_RELEASE(g_DepthStencilView);
	SAFE_RELEASE(g_RasterState);
	SAFE_RELEASE(g_DeviceContext);
#ifdef _DEBUG
	/*
	* Note the Device is still alive at this point
	*/
	g_DebugController->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	SAFE_RELEASE(g_DebugController);
#endif
	SAFE_RELEASE(g_Device);

	g_InputHandler.Shutdown();
	g_Window.Shutdown();
}