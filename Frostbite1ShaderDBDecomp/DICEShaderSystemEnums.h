#pragma once
#include "typedefs.h"


// this enum is not accurate. the values dont map to what they really are (SvtHalf3 is not 0x8, but 0x7, SvtHalf1 is not 0x0, but 0x5, etc 
enum ShaderValueType : u8
{
	SvtHalf1 = 0x0,
	SvtHalf1x2 = 0x1,
	SvtHalf1x3 = 0x2,
	SvtHalf1x4 = 0x3,
	SvtHalf2 = 0x4,
	SvtHalf2x2 = 0x5,
	SvtHalf2x3 = 0x6,
	SvtHalf2x4 = 0x7,
	SvtHalf3 = 0x8,
	SvtHalf3x2 = 0x9,
	SvtHalf3x3 = 0xA,
	SvtHalf3x4 = 0xB,
	SvtHalf4 = 0xC,
	SvtHalf4x2 = 0xD,
	SvtHalf4x3 = 0xE,
	SvtHalf4x4 = 0xF,
	SvtFloat1 = 0x10,
	SvtFloat1x2 = 0x11,
	SvtFloat1x3 = 0x12,
	SvtFloat1x4 = 0x13,
	SvtFloat2 = 0x14,
	SvtFloat2x2 = 0x15,
	SvtFloat2x3 = 0x16,
	SvtFloat2x4 = 0x17,
	SvtFloat3 = 0x18,
	SvtFloat3x2 = 0x19,
	SvtFloat3x3 = 0x1A,
	SvtFloat3x4 = 0x1B,
	SvtFloat4 = 0x1C,
	SvtFloat4x2 = 0x1D,
	SvtFloat4x3 = 0x1E,
	SvtFloat4x4 = 0x1F,
	SvtInt1 = 0x20,
	SvtInt1x2 = 0x21,
	SvtInt1x3 = 0x22,
	SvtInt1x4 = 0x23,
	SvtInt2 = 0x24,
	SvtInt2x2 = 0x25,
	SvtInt2x3 = 0x26,
	SvtInt2x4 = 0x27,
	SvtInt3 = 0x28,
	SvtInt3x2 = 0x29,
	SvtInt3x3 = 0x2A,
	SvtInt3x4 = 0x2B,
	SvtInt4 = 0x2C,
	SvtInt4x2 = 0x2D,
	SvtInt4x3 = 0x2E,
	SvtInt4x4 = 0x2F,
	SvtBool1 = 0x30,
	SvtBool1x2 = 0x31,
	SvtBool1x3 = 0x32,
	SvtBool1x4 = 0x33,
	SvtBool2 = 0x34,
	SvtBool2x2 = 0x35,
	SvtBool2x3 = 0x36,
	SvtBool2x4 = 0x37,
	SvtBool3 = 0x38,
	SvtBool3x2 = 0x39,
	SvtBool3x3 = 0x3A,
	SvtBool3x4 = 0x3B,
	SvtBool4 = 0x3C,
	SvtBool4x2 = 0x3D,
	SvtBool4x3 = 0x3E,
	SvtBool4x4 = 0x3F,
	SvtSampler2D = 0x40,
	SvtSampler3D = 0x41,
	SvtSamplerCube = 0x42,
	SvtCount = 0x43,
};


enum ShaderGeometrySpace : u8
{
	SgsObject = 0x0,
	SgsWorld = 0x1,
	SgsScreen = 0x2,
};

enum ShaderSkinningMethod : u8
{
	ShaderSkinningMethod_None = 0x0,
	ShaderSkinningMethod_1Bone = 0x1,
	ShaderSkinningMethod_2Bone = 0x2,
	ShaderSkinningMethod_4Bone = 0x4,
	ShaderSkinningMethod_8Bone = 0x8,
	ShaderSkinningMethod_Null = 0x9,
};

enum ShaderInstancingMethod : u8
{
	ShaderInstancingMethod_None = 0x0,
	ShaderInstancingMethod_WorldTransform4x3Float = 0x1,
	ShaderInstancingMethod_WorldTransform4x3Half = 0x2,
};


enum ShaderSolutionFlags : __int32
{
	Transparent = 0x1,
	TransparentDecal = 0x2,
	TransparentDepthWrite = 0x4,
	Offseted = 0x8,
	AlphaTest = 0x10,
	AlphaToMask = 0x20,
	DoubleSided = 0x40,
	DepthEnable = 0x80,
	DepthWriteEnable = 0x100,
	BlendLerp = 0x400,
	BlendAdditive = 0x800,
	BlendSubtractive = 0x1000,
	BlendRevSubtractive = 0x2000,
	BlendMultiply = 0x4000,
	BlendMin = 0x8000,
	BlendMax = 0x10000,
	BlendAlphaLerp = 0x20000,
	BlendAlphaAdditive = 0x40000,
	BlendAlphaSubtractive = 0x80000,
	BlendAlphaRevSubtractive = 0x100000,
	BlendAlphaMultiply = 0x200000,
	BlendAlphaMin = 0x400000,
	BlendAlphaMax = 0x800000,
	AllBlendColor = 0x1FC00,
	AllBlendAlpha = 0xFE0000,
	AllBlend = 0xFFFC00,
};


enum ShaderRenderMode : u8
{
	ShaderRenderMode_Normal = 0x0,
	ShaderRenderMode_Shadowbuffer = 0x1,
	ShaderRenderMode_ShadowbufferZTrans = 0x2,
	ShaderRenderMode_Shadowmap = 0x3,
	ShaderRenderMode_PointShadowbuffer = 0x4,
	ShaderRenderMode_ZOnly = 0x5,
	ShaderRenderMode_Velocity = 0x6,
	ShaderRenderMode_Simple = 0x7,
	ShaderRenderMode_SimpleLighting = 0x8,
	ShaderRenderMode_Overdraw = 0x9,
};

enum ShaderShadowmapMethod : u8
{
	ShaderShadowmapMethod_None = 0x0,
	ShaderShadowmapMethod_ManualPcf = 0x1,
	ShaderShadowmapMethod_HardwarePcf = 0x2,
	ShaderShadowmapMethod_CascadedBox3 = 0x3,
	ShaderShadowmapMethod_CascadedBox4 = 0x4,
	ShaderShadowmapMethodCount = 0x5,
};


// CONSTANT BUFFER COMMANDS ENUM :
enum ShaderConstantFunction : u8
{
	ScfViewMatrix = 0x0,
	ScfViewMatrixTranspose = 0x1,
	ScfViewProjMatrix = 0x2,
	ScfViewProjMatrixTranspose = 0x3,
	ScfWorldViewMatrix = 0x4,
	ScfWorldViewMatrixTranspose = 0x5,
	ScfWorldViewProjMatrix = 0x6,
	ScfWorldViewProjMatrixTranspose = 0x7,
	ScfWorldMatrix = 0x8,
	ScfWorldMatrixTranspose = 0x9,
	ScfSpriteMatrix = 0xA,
	ScfSpriteMatrixTranspose = 0xB,
	ScfPrevViewMatrix = 0xC,
	ScfPrevViewMatrixTranspose = 0xD,
	ScfPrevViewProjMatrix = 0xE,
	ScfPrevViewProjMatrixTranspose = 0xF,
	ScfPrevWorldViewProjMatrix = 0x10,
	ScfPrevWorldViewProjMatrixTranspose = 0x11,
	ScfPrevWorldMatrix = 0x12,
	ScfPrevWorldMatrixTranspose = 0x13,
	ScfPrevSpriteMatrix = 0x14,
	ScfPrevSpriteMatrixTranspose = 0x15,
	ScfViewportZMinMax = 0x16,
	ScfBoneMatrices = 0x17,
	ScfCameraPos = 0x18,
	ScfTime = 0x19,
	ScfInvScreenSize = 0x1A,
	ScfPointLightPos = 0x1B,
	ScfPointLightColor = 0x1C,
	ScfPointLightRadius = 0x1D,
	ScfOutdoorLightDir = 0x1E,
	ScfOutdoorLightHemisphereDir = 0x1F,
	ScfOutdoorLightKeyColor = 0x20,
	ScfOutdoorLightKeySpecularColorAndHemisphereVisibility = 0x21,
	ScfOutdoorLightTopColor = 0x22,
	ScfOutdoorLightBottomColor = 0x23,
	ScfOutdoorLightShadowTransform = 0x24,
	ScfOutdoorLightShadowmapSizeAndInvSize = 0x25,
	ScfOutdoorLightShadowmapTexture = 0x26,
	ScfOutdoorLightReflectionEnvmap = 0x27,
	ScfOutdoorLightCloudShadowInvScaleAndIntensity = 0x28,
	ScfFogStartAndEnd = 0x29,
	ScfFogColorAndClamp = 0x2A,
	ScfFogColorSunAndPower = 0x2B,
	ScfFogHeightFalloffAndOffset = 0x2C,
	ScfTransparentStartAndEndAndClamp = 0x2D,
	ScfLightScatteringSkyBetaR = 0x2E,
	ScfLightScatteringSkyBetaM = 0x2F,
	ScfLightScatteringSunDirection = 0x30,
	ScfLightScatteringSunIrradianceAndEccentrity = 0x31,
	ScfLightScatteringEarthRadiusAndZenithHeightAndHazeHeight = 0x32,
	ScfLightScatteringFogDensityAndEccentrity = 0x33,
	ScfOffsetTexture = 0x34,
	ScfColorScale = 0x35,
	ScfDebugNonFiniteColor = 0x36,
	ScfXenonInstancingIndexCountAndStart = 0x37,
	ScfPs3ClipPlane = 0x38,
	ShaderConstantFunctionCount = 0x39,
};



// VERTEX BUFFER USAGE ENUM :
enum ShaderVertexElementUsage : u8
{
	ShaderVertexElementUsage_Unknown = 0x0,
	ShaderVertexElementUsage_Pos = 0x1,
	ShaderVertexElementUsage_BoneIndices = 0x2,
	ShaderVertexElementUsage_BoneWeights = 0x3,
	ShaderVertexElementUsage_Normal = 0x4,
	ShaderVertexElementUsage_Tangent = 0x5,
	ShaderVertexElementUsage_Binormal = 0x6,
	ShaderVertexElementUsage_BinormalSign = 0x7,
	ShaderVertexElementUsage_WorldTrans1 = 0x8,
	ShaderVertexElementUsage_WorldTrans2 = 0x9,
	ShaderVertexElementUsage_WorldTrans3 = 0xA,
	ShaderVertexElementUsage_XenonIndex = 0xB,
};


enum MeshSubsetCategory : __int32
{
	MscOpaque = 0x0,
	MscTransparent = 0x1,
	MscTransparentDecal = 0x2,
	MscZOnly = 0x3,
	MscCount = 0x4,
};