//
// Texture loaders
// Adapted from:
// https://github-wiki-see.page/m/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
//

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

HRESULT LoadTextureFromFile(
    ID3D11Device* dxdevice,
    const char* filename,
    Texture* texture_out)
{
    return LoadTextureFromFile(
        dxdevice,
        nullptr,
        filename,
        texture_out);
}

HRESULT LoadTextureFromFile(
    ID3D11Device* dxdevice,
    ID3D11DeviceContext* dxdevice_context,
    const char* filename,
    Texture* texture_out)
{
    int mipLevels = 1;
    int mipLevels_srv = 1;
    unsigned bindFlags = D3D11_BIND_SHADER_RESOURCE;
    unsigned miscFlags = 0;
    int mostDetailedMip = 0;
    
    bool useMipMap = (bool)dxdevice_context;
    // Generate mip hierarchy if a dxdevice_context is provided
    if (useMipMap)
    {
        mipLevels = 0;
        mipLevels_srv = -1;
        bindFlags |= D3D11_BIND_RENDER_TARGET;
        miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        mostDetailedMip = -1;
    }

    HRESULT hr;

    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == nullptr)
    {
        return E_FAIL;
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
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
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    D3D11_SUBRESOURCE_DATA* subResourcePtr = &subResource;
    if (useMipMap) subResourcePtr = nullptr;
    if (FAILED(hr = dxdevice->CreateTexture2D(
        &desc,
        subResourcePtr,
        &pTexture)))
    {
        return hr;
    }
    SETNAME(pTexture, "TextureData");

    if (useMipMap)
        dxdevice_context->UpdateSubresource(
            pTexture,
            0,
            0,
            image_data,
            subResource.SysMemPitch,
            0);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = mipLevels_srv;
    if (FAILED(hr = dxdevice->CreateShaderResourceView(
        pTexture,
        &srvDesc,
        &texture_out->texture_SRV)))
    {
        return hr;
    }
    SETNAME((texture_out->texture_SRV), "TextureSRV");

    if (useMipMap)
        dxdevice_context->GenerateMips(texture_out->texture_SRV);

    // Cleanup
    pTexture->Release();
    stbi_image_free(image_data);

    // Done
    texture_out->width = image_width;
    texture_out->height = image_height;
    return S_OK;
}

HRESULT LoadCubeTextureFromFile(
    ID3D11Device* dxdevice,
    const char** filenames,
    Texture* texture_out)
{
    HRESULT hr;

    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data[6];
    for (int i = 0; i < 6; i++)
    {
        image_data[i] = stbi_load(filenames[i], &image_width, &image_height, NULL, 4);
        if (image_data[i] == nullptr)
        {
            return E_FAIL;
        }
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
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
        subResource[i].pSysMem = image_data[i];
        subResource[i].SysMemPitch = image_width * 4;
        subResource[i].SysMemSlicePitch = 0;
    }
    if (FAILED(hr = dxdevice->CreateTexture2D(&desc, &subResource[0], &pTexture)))
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
    if (FAILED(hr = dxdevice->CreateShaderResourceView(
        pTexture, 
        &srvDesc,
        &texture_out->texture_SRV)))
    {
        return hr;
    }
    SETNAME((texture_out->texture_SRV), "TextureSRV");

    // Cleanup
    pTexture->Release();
    for (int i = 0; i < 6; i++)
        stbi_image_free(image_data[i]);

    // Done
    texture_out->width = image_width;
    texture_out->height = image_height;
    return S_OK;
}