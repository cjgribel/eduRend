
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
#define FORCE_DGPU // Force to use the dGPU on systems with multiple adapters


#include <dxcore.h>
#include <dxgi1_6.h>
#include <array>
#include "stdafx.h"
#include "shader.h"
#include "Window.h"
#include "parseutil.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Scene.h"




//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static IDXGISwapChain*			swapChain			= nullptr;
static ID3D11RenderTargetView*	renderTargetView	= nullptr;
static ID3D11Texture2D*			depthStencil		= nullptr;
static ID3D11DepthStencilView*	depthStencilView	= nullptr;
static ID3D11Device*			device				= nullptr;
static ID3D11DeviceContext*		deviceContext		= nullptr;
static ID3D11RasterizerState*	rasterState			= nullptr;

static shader_data*				vertexShader		= nullptr;
static shader_data*				pixelShader			= nullptr;

#ifdef _DEBUG
static ID3D11Debug*				debugController		= nullptr;
#endif // _DEBUG

static const int				initialWinWidth		= 1024;
static const int				initialWinHeight	= 576;
static const size_t				DescStringSize		= 128;

static InputHandler				inputHandler;
static Window					window;
static std::unique_ptr<Scene>	scene;

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
void				Release();
void				WinResize();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
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
	window.Init(initialWinWidth, initialWinHeight);

	inputHandler.Initialize(instance, window.GetHandle(), initialWinWidth, initialWinHeight);

#ifdef USECONSOLE
	printf("Win32-window created...\n");
#ifdef _DEBUG
	printf("Running in DEBUG mode\n");
#else
	printf("Running in RELEASE mode\n");
#endif
#endif

	HRESULT hr = S_OK;

	if(SUCCEEDED(hr = InitDirect3DAndSwapChain(initialWinWidth, initialWinHeight)))
	{
		InitRasterizerState();

		if (SUCCEEDED(hr = CreateRenderTargetView()) &&
			SUCCEEDED(hr = CreateDepthStencilView(initialWinWidth, initialWinHeight)))
		{
			SetViewport(initialWinWidth, initialWinHeight);

			deviceContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

			const D3D11_INPUT_ELEMENT_DESC inputDesc[5] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			if(FAILED(create_shader(device, "shaders/vertex_shader.hlsl", "VS_main", SHADER_VERTEX, &inputDesc[0], 5, &vertexShader)))
			{
				// Can't continue the program if the shader fails to load.
				return -1;
			}
			if(FAILED(create_shader(device, "shaders/pixel_shader.hlsl", "PS_main", SHADER_PIXEL, nullptr, 0, &pixelShader)))
			{
				// Can't continue the program if the shader fails to load.
				return -1;
			}

			scene = std::make_unique<OurTestScene>(
				device,
				deviceContext,
				initialWinWidth,
				initialWinHeight);

			int64_t cps = 0;
			QueryPerformanceFrequency((LARGE_INTEGER*)&cps);
			double ss = 1.0f / (float)cps;

			int64_t start = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&start);

			scene->Init();

			int64_t end = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&end);
			double dt = ((double)end - start) * ss;
			printf("Scene loading took %lfs\n", dt);
		}
	}

	int64_t cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	int64_t prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	

	printf("Entering main loop...\n");
	
	while (window.Update())
	{
		if (window.SizeChanged())
		{
			WinResize();
		}

		int64_t currTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
		const float deltaTime = (currTimeStamp - prevTimeStamp) * secsPerCnt;
		inputHandler.Update();
		
		Update(deltaTime);
		Render(deltaTime);

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
	const linalg::vec2i size = window.GetSize();

	printf("window resized to %i x %i\n", size.x, size.y);

	// With no swapchain there is nothing to resize
	if (!swapChain) return;

	deviceContext->OMSetRenderTargets(0, 0, 0);
	// Release all outstanding references to the swap chain's buffers.
	renderTargetView->Release();

	HRESULT hr;
	// Preserve the existing buffer count and format.
	// Automatically choose the width and height to match the client rect for HWNDs.
	ASSERT(hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

	// Get buffer and create a render-target-view.
	ID3D11Texture2D* pBuffer = nullptr;
	ASSERT(hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer)));

	ASSERT(hr = device->CreateRenderTargetView(pBuffer, nullptr, &renderTargetView));
	SETNAME(renderTargetView, "RenderTargetView");

	pBuffer->Release();
	SAFE_RELEASE(depthStencil);
	SAFE_RELEASE(depthStencilView);

	CreateDepthStencilView(size.x, size.y);
	SETNAME(renderTargetView, "RenderTargetView");
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	// Set up the viewport.
	D3D11_VIEWPORT vp{};
	vp.Width = (float)size.x;
	vp.Height = (float)size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	deviceContext->RSSetViewports(1, &vp);

	scene->OnWindowResized(size.x, size.y);
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

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)width;
	sd.BufferDesc.Height = (UINT)height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window.GetHandle(); // g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL initiatedFeatureLevel;
	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	
	
#ifdef FORCE_DGPU
	
	// Checking present graphic adapters.
	// adapter will fallback to staying as nullptr on error
	std::cout << "Listing available adapters:\n";
	size_t adapterCount = 0;
	std::vector<bool> isHardwareList;
	std::vector<bool> isIntegratedList;
	std::vector<std::array<char, DescStringSize>> drvDescList;
	std::vector<IDXCoreAdapter*> adapterList;
	IDXCoreAdapterFactory* adapterFactory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	IDXCoreAdapterList* D3D11Adapters = nullptr;
	GUID attributes[]{ DXCORE_ADAPTER_ATTRIBUTE_D3D11_GRAPHICS };

	HRESULT hr = DXCoreCreateAdapterFactory(&adapterFactory);
	if (FAILED(hr)) {
		std::cerr << "Failed to create DXCoreAdapterFactory.\n";
		goto checkFailed; }
	
	hr = adapterFactory->CreateAdapterList(_countof(attributes),
										  attributes,
										   &D3D11Adapters);
	if (FAILED(hr)) {
		std::cerr << "Failed to create Adapter list.\n";
		goto checkFailed; }
	adapterCount = D3D11Adapters->GetAdapterCount();


	// query adapters and store info about them
	for (uint32_t i = 0; i < adapterCount; ++i)
	{
		IDXCoreAdapter* candidate = nullptr;
		hr = D3D11Adapters->GetAdapter(i, &candidate);
		if (FAILED(hr))
		{
			std::cerr << "Failed to get Adapter, index " << i << ".\n";
			isHardwareList.push_back(false);
			isIntegratedList.push_back(false);
			adapterList.push_back(nullptr);
			drvDescList.push_back({});
			continue;
		}

		std::array<char, DescStringSize> drvDesc;
		size_t descStringBufferSize = sizeof(drvDesc);

		if (candidate->IsPropertySupported(DXCoreAdapterProperty::IsHardware) &&
			candidate->IsPropertySupported(DXCoreAdapterProperty::IsIntegrated) &&
			candidate->IsPropertySupported(DXCoreAdapterProperty::DriverDescription))
		{
			candidate->GetProperty(DXCoreAdapterProperty::DriverDescription, descStringBufferSize, &drvDesc);
			drvDescList.push_back(drvDesc);
			bool isHw = false;
			candidate->GetProperty( DXCoreAdapterProperty::IsHardware, &isHw);
			bool isIntegrated = false;;
			candidate->GetProperty(DXCoreAdapterProperty::IsIntegrated, &isIntegrated);
			isHardwareList.push_back(isHw);
			isIntegratedList.push_back(isIntegrated);
			adapterList.push_back(candidate);

			std::cout << "\t" << i << ": " << drvDesc.data() << "\n";
		} else
		{
			isHardwareList.push_back(false);
			isIntegratedList.push_back(false);
			adapterList.push_back(nullptr);
			drvDescList.push_back(drvDesc);
			std::cout << "\t" << i << ": Invalid Device\n";
		}	
	}
	
	// find the first non iGPU hardware adapter
	for (size_t i = 0; i < adapterCount; ++i) {
		if (isHardwareList[i] && !isIntegratedList[i]) {

			LUID adapterLuid;
			size_t luidSize = sizeof(adapterLuid);
			hr = adapterList[i]->GetProperty(DXCoreAdapterProperty::InstanceLuid, luidSize, &adapterLuid);
			if (FAILED(hr)) {
				std::cerr << "Failed to create Adapter list.\n"; break; }

			// Setup older DXGI factory and get the DXGIAdapter by LUID
			IDXGIFactory6* oldFactory = nullptr;
			hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&oldFactory));
			if (FAILED(hr)) {
				std::cerr << "Failed to create DXGIFactory.\n"; break; }

			hr = oldFactory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&adapter));
			if (FAILED(hr)) {
				std::cerr << "Failed to aquire DXGIAdapter from LUID.\n";
				adapter = nullptr;
			}
			if (oldFactory)
				oldFactory->Release();

			if (adapter)
				std::cout << "\nPicking Adapter " << i << ": " << drvDescList[i].data() << "\n\n";

			break;
		}
	}
checkFailed:
	if (D3D11Adapters) D3D11Adapters->Release();
	if (adapterFactory) adapterFactory->Release();
	for (size_t i = 0; i < adapterCount; ++i)
	{
		if (adapterList[i])
			adapterList[i]->Release();
	}
#else
	HRESULT hr;
	IDXGIAdapter* adapter = nullptr;
#endif

	if (!adapter)
		std::cout << "\nDefaulting to primary adapter.\n\n";

	hr = E_FAIL;
	for (UINT driverTypeIndex = 0;
		 driverTypeIndex < ARRAYSIZE(driverTypes) && FAILED(hr);
		 driverTypeIndex++)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			adapter, 
			adapter == nullptr ? driverTypes[driverTypeIndex] : D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			flags,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&swapChain,
			&device,
			&initiatedFeatureLevel,
			&deviceContext);
	}
#ifdef _DEBUG
	if (hr == S_OK)
	{
		device->QueryInterface(&debugController);
		SETNAME(swapChain, "Swapchain");
		SETNAME(device, "Device");
		SETNAME(deviceContext, "Context");
	}
#endif // _DEBUG
	return hr;
}

void InitRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerState{};
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

	device->CreateRasterizerState(&rasterizerState, &rasterState);
	SETNAME(rasterState, "RasterizerState");
	deviceContext->RSSetState(rasterState);
}

HRESULT CreateRenderTargetView()
{
	HRESULT hr = S_OK;
	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (SUCCEEDED(hr))
	{
		hr = device->CreateRenderTargetView( pBackBuffer, nullptr, &renderTargetView );
		SETNAME(renderTargetView, "RenderTargetView");
		SAFE_RELEASE(pBackBuffer);
	}
	return hr;
}

HRESULT CreateDepthStencilView(int width, int height)
{
	HRESULT hr = S_OK;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth{};
	descDepth.Width = (UINT)width;
	descDepth.Height = (UINT)height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = device->CreateTexture2D( &descDepth, nullptr, &depthStencil );
	if( FAILED(hr) )
		return hr;
	SETNAME(depthStencil, "DepthStencil");
	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView( depthStencil, &descDSV, &depthStencilView );
	SETNAME(depthStencilView, "DepthStencilView");
	return hr;
}

void SetViewport(int width, int height)
{
	// Setup the viewport
	D3D11_VIEWPORT vp{};
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	deviceContext->RSSetViewports( 1, &vp );
}

HRESULT Update(float deltaTime)
{
	scene->Update(deltaTime, inputHandler);

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	// Get rid of unreferenced warning
	deltaTime = deltaTime;

	// Clear color in RGBA
	static float ClearColor[4] = { 0, 0, 0, 1 };

	// Clear back buffer
	deviceContext->ClearRenderTargetView( renderTargetView, ClearColor );
	
	// Clear depth and stencil buffer
	deviceContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	
	// Set topology
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
	// Bind shaders
	bind_shader(device, deviceContext, vertexShader);
	bind_shader(device, deviceContext, pixelShader);

	// These shader types are not used
	deviceContext->HSSetShader(nullptr, nullptr, 0);
	deviceContext->DSSetShader(nullptr, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	
	// Time for the current scene to render
	scene->Render();

	// Swap front and back buffer
#ifdef VSYNC
	// Swapping synchronized with monitor
	return swapChain->Present(1, 0);
#else
	// Swapping not synchronized with monitor
	return g_SwapChain->Present(0, 0);
#endif
}

void Release()
{
	SAFE_RELEASE(scene);

	delete_shader(vertexShader);
	delete_shader(pixelShader);

	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(depthStencil);
	SAFE_RELEASE(depthStencilView);
	SAFE_RELEASE(rasterState);
	SAFE_RELEASE(deviceContext);
#ifdef _DEBUG
	/*
	* Note the Device is still alive at this point
	*/
	debugController->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	SAFE_RELEASE(debugController);
#endif
	SAFE_RELEASE(device);

	inputHandler.Shutdown();
	window.Shutdown();
}