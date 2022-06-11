//
// Texture loaders
// Adapted from:
// https://github-wiki-see.page/m/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
//

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "StbImage.h"

HRESULT LoadTextureFromFile(
    ID3D11Device* dxDevice,
    const char* fileName,
    Texture* textureOut)
{
    return LoadTextureFromFile(
        dxDevice,
        nullptr,
        fileName,
        textureOut);
}

HRESULT LoadTextureFromFile(
    ID3D11Device* dxDevice,
    ID3D11DeviceContext* dxDeviceContext,
    const char* fileName,
    Texture* textureOut)
{
    int mipLevels = 1;
    int mipLevelsSrv = 1;
    unsigned bindFlags = D3D11_BIND_SHADER_RESOURCE;
    unsigned miscFlags = 0;
    int mostDetailedMip = 0;
    
    bool useMipMap = (bool)dxDeviceContext;
    // Generate mip hierarchy if a dxDeviceContext is provided
    if (useMipMap)
    {
        mipLevels = 0;
        mipLevelsSrv = -1;
        bindFlags |= D3D11_BIND_RENDER_TARGET;
        miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        mostDetailedMip = -1;
    }

    HRESULT hr;

    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int imageWidth = 0;
    int imageHeight = 0;
    unsigned char* imageData = stbi_load(fileName, &imageWidth, &imageHeight, NULL, 4);
    if (imageData == nullptr)
    {
        return E_FAIL;
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = imageWidth;
    desc.Height = imageHeight;
    desc.MipLevels = mipLevels;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = miscFlags;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = imageData;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    D3D11_SUBRESOURCE_DATA* subResourcePtr = &subResource;
    if (useMipMap) subResourcePtr = nullptr;
    if (FAILED(hr = dxDevice->CreateTexture2D(
        &desc,
        subResourcePtr,
        &pTexture)))
    {
        return hr;
    }
    SETNAME(pTexture, "TextureData");

    if (useMipMap)
        dxDeviceContext->UpdateSubresource(
            pTexture,
            0,
            0,
            imageData,
            subResource.SysMemPitch,
            0);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = mipLevelsSrv;
    if (FAILED(hr = dxDevice->CreateShaderResourceView(
        pTexture,
        &srvDesc,
        &textureOut->shaderResourceView)))
    {
        return hr;
    }
    SETNAME((textureOut->shaderResourceView), "TextureSRV");

    if (useMipMap)
        dxDeviceContext->GenerateMips(textureOut->shaderResourceView);

    // Cleanup
    pTexture->Release();
    stbi_image_free(imageData);

    // Done
    textureOut->width = imageWidth;
    textureOut->height = imageHeight;
    return S_OK;
}

HRESULT LoadCubeTextureFromFile(
    ID3D11Device* dxDevice,
    const char** fileNames,
    Texture* textureOut)
{
    HRESULT hr;

    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int imageWidth = 0;
    int imageHeight = 0;
    unsigned char* imageData[6];
    for (int i = 0; i < 6; i++)
    {
        imageData[i] = stbi_load(fileNames[i], &imageWidth, &imageHeight, NULL, 4);
        if (imageData[i] == nullptr)
        {
            return E_FAIL;
        }
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = imageWidth;
    desc.Height = imageHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource[6];
    for (int i = 0; i < 6; i++)
    {
        subResource[i].pSysMem = imageData[i];
        subResource[i].SysMemPitch = imageWidth * 4;
        subResource[i].SysMemSlicePitch = 0;
    }
    if (FAILED(hr = dxDevice->CreateTexture2D(&desc, &subResource[0], &pTexture)))
    {
        return hr;
    }
    SETNAME(pTexture, "TextureData");

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(hr = dxDevice->CreateShaderResourceView(
        pTexture, 
        &srvDesc,
        &textureOut->shaderResourceView)))
    {
        return hr;
    }
    SETNAME((textureOut->shaderResourceView), "TextureSRV");

    // Cleanup
    pTexture->Release();
    for (int i = 0; i < 6; i++)
        stbi_image_free(imageData[i]);

    // Done
    textureOut->width = imageWidth;
    textureOut->height = imageHeight;
    return S_OK;
}