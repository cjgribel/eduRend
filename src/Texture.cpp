
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Simple helper function to load an image into a DX11 texture with common settings
// https://github-wiki-see.page/m/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
//
bool LoadTextureFromFile(
    ID3D11Device* dxdevice,
    const char* filename, 
    ID3D11ShaderResourceView** out_srv, 
    int* out_width, 
    int* out_height)
{
    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == nullptr)
        throw std::runtime_error("Failed to load texture!");

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    if (FAILED(dxdevice->CreateTexture2D(&desc, &subResource, &pTexture)))
        throw std::runtime_error("Failed to load " + std::string(filename));
    SETNAME(pTexture, "TextureData");
    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(dxdevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv)))
    {
        throw std::runtime_error("Failed to create ShaderResourceView");
    }
    SETNAME((*out_srv), "TextureSRV");
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

bool LoadCubeTextureFromFile(
    ID3D11Device* dxdevice,
    const char** filenames,
    ID3D11ShaderResourceView** out_srv,
    int* out_width,
    int* out_height)
{
    // Load from disk into a raw RGBA buffer
    stbi_set_flip_vertically_on_load(1);
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data[6];
    for (int i = 0; i < 6; i++)
    {
        image_data[i] = stbi_load(filenames[i], &image_width, &image_height, NULL, 4);
        if (image_data[i] == nullptr)
            throw std::runtime_error("Failed to load " + std::string(filenames[i]));
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
    if (FAILED(dxdevice->CreateTexture2D(&desc, &subResource[0], &pTexture)))
    {
        throw std::runtime_error("Failed to create Texture2D");
    }
    SETNAME(pTexture, "TextureData");
    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(dxdevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv)))
    {
        throw std::runtime_error("Failed to create ShaderResourceView");
    }
    SETNAME((*out_srv), "TextureSRV");
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    for (int i = 0; i < 6; i++)
        stbi_image_free(image_data[i]);

    return true;
}