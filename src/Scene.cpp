
#include "Scene.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	dxdevice(dxdevice),
	dxdevice_context(dxdevice_context),
	window_width(window_width),
	window_height(window_height)
{ }

void Scene::Init()
{
}

void Scene::Update(
	float dt,
	InputHandler* input_handler)
{}

void Scene::Render()
{
	// TODO: remove
	//  cube map (slot 2)
//	dxdevice_context->PSSetShaderResources(2, 1, &map_cube_TexSRV);
}

void Scene::Release()
{}

void Scene::WindowResize(
	int window_width,
	int window_height)
{}

void Scene::InitShaderBuffers()
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
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &matrix_buffer));

	// Point light buffer
	D3D11_BUFFER_DESC UBuffer_desc = { 0 };
	UBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	UBuffer_desc.ByteWidth = sizeof(UniformBuffer);
	UBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	UBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	UBuffer_desc.MiscFlags = 0;
	UBuffer_desc.StructureByteStride = 0;
	hr = dxdevice->CreateBuffer(&UBuffer_desc, nullptr, &uniform_buffer);
	ASSERT(hr);
}

void Scene::MapMatrixShaderBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	MatrixBuffer_t* matrix_buffer_ = (MatrixBuffer_t*)resource.pData;
	matrix_buffer_->ModelToWorldMatrix = ModelToWorldMatrix;
	matrix_buffer_->WorldToViewMatrix = WorldToViewMatrix;
	matrix_buffer_->ProjectionMatrix = ProjectionMatrix;
	dxdevice_context->Unmap(matrix_buffer, 0);

	// attach matrix buffers (updated per object)
	dxdevice_context->VSSetConstantBuffers(0, 1, &matrix_buffer);
}

void Scene::MapUniformBuffer(
	const vec3f& eyePos,
	const vec3f& lightPos)
{
	// update light buffer
	D3D11_MAPPED_SUBRESOURCE ures;
	dxdevice_context->Map(uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ures);
	UniformBuffer* uniformBuffer = (UniformBuffer*)ures.pData;
	uniformBuffer->eyePos = eyePos; // camera->get_position();
	uniformBuffer->lightPos = lightPos; // lightPos.xyz();
	dxdevice_context->Unmap(uniform_buffer, 0);
	
	// attach light buffer
	dxdevice_context->PSSetConstantBuffers(0, 1, &uniform_buffer);
}

// OBJModel_t::render()
// 		MapMaterialBuffer(mtl);
// bind it
//dxdevice_context->PSSetConstantBuffers(1, 1, &MaterialBuffer);

//
// Declarations
//

// Objects
camera_t* camera;

QuadModel* quad;
OBJModel* sponza;

// Object model-to-world transformation matrices and stuff related to them
mat4f Msponza;
mat4f Mquad;

// World-to-view matrix
mat4f Mview;
// Projection matrix
mat4f Mproj;

// Misc
float angle = 0;			// A per-frame updated rotation angle (radians)...
float angle_vel = fPI / 2;	// ...and its velocity (radians/sec)
float camera_vel = 5.0f;	// Camera movement velocity in units/s
float fps_cooldown = 0;

// TOD: remove
CubeTexture cube_texture;

//
// Called at initialization
//
void initObjects(
	unsigned window_width,
	unsigned window_height,
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context)
{
	// Create camera
	camera = new camera_t(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)window_width / window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	camera->moveTo({ 0, 0, 5 });

	// Create objects
	quad = new QuadModel(dxdevice, dxdevice_context);
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);

	// TODO: remove
	cube_texture.filenames[0] = "assets/cubemaps/debug_cubemap/debug_posx.png";
	cube_texture.filenames[1] = "assets/cubemaps/debug_cubemap/debug_negx.png";
	cube_texture.filenames[2] = "assets/cubemaps/debug_cubemap/debug_posy.png";
	cube_texture.filenames[3] = "assets/cubemaps/debug_cubemap/debug_negy.png";
	cube_texture.filenames[4] = "assets/cubemaps/debug_cubemap/debug_posz.png";
	cube_texture.filenames[5] = "assets/cubemaps/debug_cubemap/debug_negz.png";
	try {
		LoadCubeTextureFromFile(dxdevice, cube_texture.filenames, &cube_texture.texture_srv, &cube_texture.width, &cube_texture.height);
	}
	catch (...) {
		printf("FAILED\n");
	}
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void updateObjects(
	float dt, 
	InputHandler* input_handler)
{
	// basic camera control
	if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->move({ 0.0f, 0.0f, -camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->move({ 0.0f, 0.0f, camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->move({ camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->move({ -camera_vel * dt, 0.0f, 0.0f });
	
	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	Mquad = mat4f::translation(0, 0, 0) *			// No translation
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%

	// Sponza model-to-world transformation
	Msponza = mat4f::translation(0, -5, 0) *		 // Move down 5 units
		mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		mat4f::scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	// Increase the rotation angle.
	angle += angle_vel * dt;

	// Print fps
	if (fps_cooldown < 0.0)
	{
		printf("fps %i\n", (int)(1.0f / dt));
		fps_cooldown = 2.0;
	}
	else
		fps_cooldown -= dt;
}

//
// Called every frame, after update
//
void renderObjects(ID3D11Buffer* matrix_buffer,
	ID3D11DeviceContext* dxdevice_context)
{
	// TODO: remove
//  cube map (slot 2)
	dxdevice_context->PSSetShaderResources(2, 1, &cube_texture.texture_srv);

	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	quad->MapMatrixBuffers(matrix_buffer, Mquad, Mview, Mproj);
	quad->Render();

	// Load matrices + Sponza's transformation to the device and render it
	sponza->MapMatrixBuffers(matrix_buffer, Msponza, Mview, Mproj);
	sponza->Render();
}

//
// Called when window is resized
//
void WindowResize(int width, int height)
{
	if (camera)
		camera->aspect = float(width) / height;
}

//
// Called at termination
//
void releaseObjects()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(sponza);
	SAFE_DELETE(camera);
}