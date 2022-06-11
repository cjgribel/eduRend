//
//  Model.cpp
//
//  CJ Gribel 2016, cjgribel@gmail.com
//

#include "Model.h"

QuadModel::QuadModel(ID3D11Device* dxDevice, ID3D11DeviceContext* dxDeviceContext)
	: Model(dxDevice, dxDeviceContext)
{
	// Vertex and index arrays
	// Once their data is loaded to GPU buffers, they are not needed anymore
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	// Populate the vertex array with 4 vertices
	Vertex v0, v1, v2, v3;
	v0.pos = { -0.5, -0.5f, 0.0f };
	v0.normal = { 0, 0, 1 };
	v0.texCoord = { 0, 0 };
	v1.pos = { 0.5, -0.5f, 0.0f };
	v1.normal = { 0, 0, 1 };
	v1.texCoord = { 0, 1 };
	v2.pos = { 0.5, 0.5f, 0.0f };
	v2.normal = { 0, 0, 1 };
	v2.texCoord = { 1, 1 };
	v3.pos = { -0.5, 0.5f, 0.0f };
	v3.normal = { 0, 0, 1 };
	v3.texCoord = { 1, 0 };

	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	// Populate the index array with two triangles

	// Triangle #1
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);
	// Triangle #2
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(vertices.size()*sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vdata;
	vdata.pSysMem = &vertices[0];
	// Create vertex buffer on device using descriptor & data
	const HRESULT vhr = dxDevice->CreateBuffer(&vbufferDesc, &vdata, &vertexBuffer);
	SETNAME(vertexBuffer, "VertexBuffer");
    
	// Index array descriptor
	D3D11_BUFFER_DESC ibufferDesc = { 0 };
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = 0;
	ibufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufferDesc.MiscFlags = 0;
	ibufferDesc.ByteWidth = (UINT)(indices.size()*sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	const HRESULT ihr = dxDevice->CreateBuffer(&ibufferDesc, &idata, &indexBuffer);
	SETNAME(indexBuffer, "IndexBuffer");
    
	numberOfIndices = (unsigned int)indices.size();
}

void QuadModel::Render() const
{
	// Bind our vertex buffer
	const UINT32 stride = sizeof(Vertex); //  sizeof(float) * 8;
	const UINT32 offset = 0;
	dxDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Bind our index buffer
	dxDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Make the drawcall
	dxDeviceContext->DrawIndexed(numberOfIndices, 0, 0);
}

OBJModel::OBJModel(const std::string& objFile, ID3D11Device* dxDevice, ID3D11DeviceContext* dxDeviceContext)
	: Model(dxDevice, dxDeviceContext)
{
	// Load the OBJ
	OBJLoader* mesh = new OBJLoader();
	mesh->Load(objFile);

	// Load and organize indices in ranges per drawcall (material)

	std::vector<unsigned> indices;
	unsigned int i_ofs = 0;

	for (auto& drawcall : mesh->drawcalls)
	{
		// Append the drawcall indices
		for (auto& tri : drawcall.tris)
			indices.insert(indices.end(), tri.vi, tri.vi + 3);

		// Create a range
		unsigned int i_size = (unsigned int)drawcall.tris.size() * 3;
		int materialIndex = drawcall.materialIndex > -1 ? drawcall.materialIndex : -1;
		indexRanges.push_back({ i_ofs, i_size, 0, materialIndex });

		i_ofs = (unsigned int)indices.size();
	}

	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(mesh->vertices.size()*sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vdata;
	vdata.pSysMem = &(mesh->vertices)[0];
	// Create vertex buffer on device using descriptor & data
	HRESULT vhr = dxDevice->CreateBuffer(&vbufferDesc, &vdata, &vertexBuffer);
	SETNAME(vertexBuffer, "VertexBuffer");
    
	// Index array descriptor
	D3D11_BUFFER_DESC ibufferDesc = { 0 };
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = 0;
	ibufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufferDesc.MiscFlags = 0;
	ibufferDesc.ByteWidth = (UINT)(indices.size()*sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	HRESULT ihr = dxDevice->CreateBuffer(&ibufferDesc, &idata, &indexBuffer);
	SETNAME(indexBuffer, "IndexBuffer");
    
	// Copy materials from mesh
	AppendMaterials(mesh->materials);

	// Go through materials and load textures (if any) to device
	std::cout << "Loading textures..." << std::endl;

	for (auto& material : materials)
	{
		HRESULT hr;

		// Load Diffuse texture
		if (material.diffuseTextureFileName.size())
		{
			hr = LoadTextureFromFile(
				dxDevice,
				material.diffuseTextureFileName.c_str(), 
				&material.diffuseTexture);

			std::cout << "\t" << material.diffuseTextureFileName 
				<< (SUCCEEDED(hr) ? " - OK" : " - FAILED") << std::endl;
		}

		// + other texture types here - see Material class
		// ...
	}

	std::cout << "Done." << std::endl;
	SAFE_DELETE(mesh);
}

void OBJModel::Render() const
{
	// Bind vertex buffer
	const UINT32 stride = sizeof(Vertex);
	const UINT32 offset = 0;
	dxDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Bind index buffer
	dxDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Iterate drawcalls
	for (auto& indexRange : indexRanges)
	{
		// Fetch material
		const Material& material = materials[indexRange.materialIndex];

		// Bind diffuse texture to slot t0 of the PS
		dxDeviceContext->PSSetShaderResources(0, 1, &material.diffuseTexture.shaderResourceView);
		// + bind other textures here, e.g. a normal map, to appropriate slots

		// Make the drawcall
		dxDeviceContext->DrawIndexed(indexRange.size, indexRange.start, 0);
	}
}

OBJModel::~OBJModel()
{
	for (auto& material : materials)
	{
		SAFE_RELEASE(material.diffuseTexture.shaderResourceView);

		// Release other used textures ...
	}
}