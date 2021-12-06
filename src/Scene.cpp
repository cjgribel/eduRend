
#include "Scene.h"

//
// Declarations
//

// Objects
camera_t* camera;

Quad_t* quad;
OBJModel_t* sponza;

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
	quad = new Quad_t(dxdevice, dxdevice_context);
	sponza = new OBJModel_t("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);
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
void renderObjects(ID3D11Buffer* matrix_buffer)
{
	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	quad->MapMatrixBuffers(matrix_buffer, Mquad, Mview, Mproj);
	quad->render();

	// Load matrices + Sponza's transformation to the device and render it
	sponza->MapMatrixBuffers(matrix_buffer, Msponza, Mview, Mproj);
	sponza->render();
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