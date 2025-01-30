#include <cstdint>
#include <iostream>
#include <vector>
#include <array>
#include <dxcore.h>
#include <dxgi1_6.h>
#include <dxgi.h>
#include <utility>

namespace DGpuForcer
{
	static std::pair<HRESULT, IDXGIAdapter*> GetAdapterPtr() {

		static const size_t	DescStringSize = 128;
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
			goto checkFailed;
		}

		hr = adapterFactory->CreateAdapterList(_countof(attributes),
											   attributes,
											   &D3D11Adapters);
		if (FAILED(hr)) {
			std::cerr << "Failed to create Adapter list.\n";
			goto checkFailed;
		}
		adapterCount = D3D11Adapters->GetAdapterCount();


		// query adapters and store info about them
		for (uint32_t i = 0; i < adapterCount; ++i) {
			IDXCoreAdapter* candidate = nullptr;
			hr = D3D11Adapters->GetAdapter(i, &candidate);
			if (FAILED(hr)) {
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
				candidate->IsPropertySupported(DXCoreAdapterProperty::DriverDescription)) {
				candidate->GetProperty(DXCoreAdapterProperty::DriverDescription, descStringBufferSize, &drvDesc);
				drvDescList.push_back(drvDesc);
				bool isHw = false;
				candidate->GetProperty(DXCoreAdapterProperty::IsHardware, &isHw);
				bool isIntegrated = false;;
				candidate->GetProperty(DXCoreAdapterProperty::IsIntegrated, &isIntegrated);
				isHardwareList.push_back(isHw);
				isIntegratedList.push_back(isIntegrated);
				adapterList.push_back(candidate);

				std::cout << "\t" << i << ": " << drvDesc.data() << "\n";
			} else {
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
					std::cerr << "Failed to create Adapter list.\n"; break;
				}

				// Setup older DXGI factory and get the DXGIAdapter by LUID
				IDXGIFactory6* oldFactory = nullptr;
				hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&oldFactory));
				if (FAILED(hr)) {
					std::cerr << "Failed to create DXGIFactory.\n"; break;
				}

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
		for (size_t i = 0; i < adapterCount; ++i) {
			if (adapterList[i])
				adapterList[i]->Release();
		}

		return std::make_pair(hr, adapter);
	}
}