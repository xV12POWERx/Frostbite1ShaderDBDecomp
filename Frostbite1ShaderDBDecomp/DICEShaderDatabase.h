#pragma once
#include "DICEShaderSystem.h"

typedef struct ShaderDatabase;

struct ShaderDatabase
{
	// storage
	Dx11PixelShaderPermutation* pixelShaders;
	Dx11VertexShaderPermutation* vertexShaders;
	DICEGeometryDeclaration* geometryDeclarations;
	ShaderStruct* shaders;
};
