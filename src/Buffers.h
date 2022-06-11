
#pragma once
#ifndef BUFFERS_H
#define BUFFERS_H

#include "vec\Vec.h"
#include "vec\Mat.h"

using namespace linalg;

// 
// CBuffer client-side definitions
// These must match the corresponding shader definitions 
//

// Struct equivalent of CBuffer "TransformationBuffer" in "vertex_shader.hlsl"
struct TransformationBuffer
{
	Mat4f modelToWorldMatrix;
	Mat4f worldToViewMatrix;
	Mat4f projectionMatrix;
};

// + more structs

#endif