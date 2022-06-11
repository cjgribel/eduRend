
#include "Scene.h"
#include "Buffers.h"

Scene::Scene(
	ID3D11Device* dxDevice,
	ID3D11DeviceContext* dxDeviceContext,
	int windowWidth,
	int windowHeight) :
	dxDevice(dxDevice),
	dxDeviceContext(dxDeviceContext),
	windowWidth(windowWidth),
	windowHeight(windowHeight)
{ }

void Scene::ResizeWindow(int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxDevice,
	ID3D11DeviceContext* dxDeviceContext,
	int windowWidth,
	int windowHeight) :
	Scene(dxDevice, dxDeviceContext, windowWidth, windowHeight)
{ 
	InitTransformationBuffer();
	// + init other CBuffers
}

//
// Called once at initialization
//
void OurTestScene::Init()
{
	camera = new Camera(
		45.0f * fTO_RAD,		           // field-of-view (radians)
		(float)windowWidth / windowHeight, // aspect ratio
		1.0f,					           // z-near plane (everything closer will be clipped/removed)
		500.0f,	                		   // z-far plane (everything further will be clipped/removed)
		10.0f);                            // Movement speed

	// Move camera to (0,0,5)
	camera->MoveTo({ 0, 0, 5 });

	// Create objects
	quad = new QuadModel(dxDevice, dxDeviceContext);
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxDevice, dxDeviceContext);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(float deltaTime, InputHandler* inputHandler)
{
	// Update camera
	camera->Update(inputHandler, deltaTime);

	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. mtwQuad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	mtwQuad = Mat4f::Translation(0, 0, 0) *			// No translation
		Mat4f::Rotation(-angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		Mat4f::Scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%

	// Sponza model-to-world transformation
	mtwSponza = Mat4f::Translation(0, -5, 0) *		 // Move down 5 units
		Mat4f::Rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		Mat4f::Scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	// Increment the rotation angle.
	angle += angularVelocity * deltaTime;

	// Print fps
	fpsCooldown -= deltaTime;
	if (fpsCooldown < 0.0)
	{
		std::cout << "fps " << (int)(1.0f / deltaTime) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		fpsCooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	// Bind transformationBuffer to slot b0 of the VS
	dxDeviceContext->VSSetConstantBuffers(0, 1, &transformationBuffer);

	// Obtain the matrices needed for rendering from the camera
	worldToViewMatrix = camera->Get_WorldToViewMatrix();
	projectionMatrix = camera->Get_ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	UpdateTransformationBuffer(mtwQuad, worldToViewMatrix, projectionMatrix);
	quad->Render();

	// Load matrices + Sponza's transformation to the device and render it
	UpdateTransformationBuffer(mtwSponza, worldToViewMatrix, projectionMatrix);
	sponza->Render();
}

void OurTestScene::Release()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(sponza);
	SAFE_DELETE(camera);

	SAFE_RELEASE(transformationBuffer);
	// + release other CBuffers
}

void OurTestScene::ResizeWindow(int width, int height)
{
	if (camera)
		camera->aspect = float(width) / height;

	Scene::ResizeWindow(width, height);
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(TransformationBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	ASSERT(hr = dxDevice->CreateBuffer(&bufferDesc, nullptr, &transformationBuffer));
}

void OurTestScene::UpdateTransformationBuffer(
	Mat4f modelToWorldMatrix,
	Mat4f worldToViewMatrix,
	Mat4f projectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxDeviceContext->Map(transformationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* structBuffer = (TransformationBuffer*)resource.pData;
	structBuffer->modelToWorldMatrix = modelToWorldMatrix;
	structBuffer->worldToViewMatrix = worldToViewMatrix;
	structBuffer->projectionMatrix = projectionMatrix;
	dxDeviceContext->Unmap(transformationBuffer, 0);
}