#pragma once
#include "typedefs.h"
#include "DICEShaderSystemEnums.h"
#include <wrl.h>
#include <d3d11.h>
#include "fb_array.h"

typedef struct PointLightSolutionState;
typedef struct OutdoorLightSolutionState;
typedef struct ShaderSolutionState;
typedef struct ShaderSolution; 
typedef struct Dx11VertexShaderPermutation; 
typedef struct Dx11PixelShaderPermutation;
typedef struct ShaderConstants;
typedef struct ShaderConstantFunctionBlockConstant;
typedef struct ShaderConstantFunctionBlock;
typedef struct ShaderConstantsExternalValueConstant;
typedef struct ShaderConstantsExternalTextureConstant;
typedef struct ShaderConstantsSamplerState;
typedef struct ShaderConstantsTextureConstant;
typedef struct ShaderTextureFunctionBlockTexture;
typedef struct ShaderTextureFunctionBlock; 
typedef struct ShaderStruct; 
typedef struct FragmentShaderStruct; 
typedef struct GeometryReferenceStruct; 
typedef struct DICEGeometryDeclaration;

struct ShaderStruct
{
	char id[16];
	char* name; 
	u8 isOriginal; 
	i32 nSolutions; 
	fb_array solutions; 
	ShaderStruct* next; 
};

struct FragmentShaderStruct
{
	char* name; 
	fb_array availableIds; 
};

struct GeometryReferenceStruct
{
	char geometryDeclarationId[16];
	char* name;
};

struct GeometryDeclarationElement
{
	ShaderValueType format;
	ShaderVertexElementUsage usage;
	u8 offset;
	u8 streamIndex;
};

struct DICEGeometryDeclaration
{
	char guid[16]; 
	GeometryDeclarationElement elements[16];
	u8 elementCount;
	u8 streamClassifications[4];
	u8 streamStrides[4];
	DICEGeometryDeclaration* next;
};


// PERMUTATION BLOCK DATA :
struct PointLightSolutionState
{
	u8 diffuseEnable;
	u8 specularEnable;
	u8 shadowbuffer;
	u8 shadowmap;
	u8 texture;
	u8 shadowmapMethod;
};

struct OutdoorLightSolutionState
{
	u8 enable;
	u8 shadowmapMethod;
};

struct ShaderSolutionState
{
	char surfaceShaderId[16];
	char vertexShaderFragmentId[16];
	char geometryDeclarationId[16];
	u8 mode;
	u8 scaleable;
	u8 geometrySpace;
	u8 skinningMethod;
	u8 instancingMethod;
	u8 fog;
	u8 colorScale;
	u8 technique;
	u8 pointLightCount;
	u8 customBonesEnable;
	u8 ps3ClipPlaneCount;
	u8 pad[3];
	PointLightSolutionState pointLights[4];
	OutdoorLightSolutionState outdoorLight;
};

struct ShaderSolution {
	// this is what goes into the serialized database 
	char id[16];
	u32 flags;
	u32 alphaRef;
	u32 hash;
	u32 null;
	u32 vsIndex;
	u32 psIndex;
	u32 vertexShaderResourceIndex; // this is the empty one 
	u32 pixelShaderResourceIndex;
	ShaderSolutionState* solutionState; 
	//
	Dx11VertexShaderPermutation* vertexShader;
	Dx11PixelShaderPermutation* pixelShader;
	ShaderConstants* vertexShaderResourceOverride;
	ShaderConstants* pixelShaderResourceOverride;
	ShaderSolution* next;
};

struct ShaderConstantsTextureConstant //this is shader_rsrc_t in shader_db.h
{
	u8 index; // was originally u8 in the pdb. fuck the rest
	u8 type;
	u16 pad;
	char textureName[128];
	// added by me 
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTXT;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTView;
};

struct ShaderConstantsSamplerState // this is shader_rsrc_s in shader_db.h
{
	u32 index;
	D3D11_SAMPLER_DESC sd; 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSS;
};


struct ShaderConstantsExternalValueConstant // this is shader_rsrc_p in shader_db.h 
{
	char name[32];
	u32 handle;
	u8 index;
	u8 size;
	u8 required;
	u8 pad;
	float defaultValue[4];
};


struct ShaderConstantsExternalTextureConstant // this is shader_rsrc_i in shader_db.h 
{
	char name[32];
	u32 handle;
	u16 index;
	u8 type; // TextureType
	u8 pad;
};



struct ShaderConstants 
{
	ShaderConstantsTextureConstant* textureConstants; 
	ShaderConstantsExternalTextureConstant* externalTextureConstants;
	ShaderConstantsSamplerState* samplerStates;
	ShaderConstantsExternalValueConstant* externalValueConstants;
	// this is for serialization
	u32 totalSize; //size of the entity
	u32 headerSize; // "header size" (also the offset where data starts)
	u32 textureOffset; 
	u32 gameparamsOffset; // game parameter that the shader uses 
	u32 filetypeOffset; // file types that the shader uses 
	u32 samplersOffset; // samplers_section
	u8 m_constantCount;
	u8 m_valueConstantStart;
	u8 m_valueConstantCount;
	u8 m_textureConstantCount;
	u8 m_externalValueConstantCount;
	u8 m_externalTextureConstantCount;
	u8 m_samplerStateCount;
	u8 paddingByte; 
};



struct Dx11PixelShaderPermutation
{
	char id[16];
	ShaderConstantFunctionBlock* constantFunctionBlock; 
	ShaderTextureFunctionBlock* textureFunctionBlock; // this is d3d_commands_2 in shader_db.h 
	ShaderConstants* constants; // this is d3d_commands_1 in shader_db.h 
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
	// added by me 
	void* shaderBytecode;
	u32 shaderBytecodeSize;
	u32 shaderStructSize; // guid + shader code + the 3 indices/pointers to the inputs 
	u32 ScfIndex; //  ShaderConstantFunction index in shader database 
	u32 StfIndex; //  ShaderTextureFunction index in shader database 
	u32 ScIndex;  //  ShaderConstants index in shader database 
	Dx11PixelShaderPermutation* next;
};


struct Dx11VertexShaderPermutation
{
	char id[16];
	ShaderConstantFunctionBlock* constantFunctionBlock;
	ShaderTextureFunctionBlock* textureFunctionBlock; // this is d3d_commands_2 in shader_db.h 
	ShaderConstants* constants; // this is d3d_commands_1 in shader_db.h 
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	// added by me 
	void* shaderBytecode;
	void* shaderBytecodeInputSignature; 
	D3D11_INPUT_ELEMENT_DESC* inputLayoutElements; 
	u32 nInputLayoutElements;
	u32 shaderBytecodeSize;
	u32 shaderBytecodeInputSignatureSize; // guid + shader code + the 3 indices/pointers to the inputs 
	u32 totalSizeOfThisShit; 
	u32 ScfIndex; //  ShaderConstantFunction index in shader database 
	u32 StfIndex; //  ShaderTextureFunction index in shader database 
	u32 ScIndex;  //  ShaderConstants index in shader database 
	Dx11PixelShaderPermutation* next;
};


struct ShaderConstantFunctionBlockConstant
{
	ShaderConstantFunction function;
	u8 index;
	u8 parameter;
	u8 arraySize;
};

struct ShaderConstantFunctionBlock
{
	u16 constantCount;
	u16 registerCount;
	ShaderConstantFunctionBlockConstant* constants;
	ShaderConstantFunctionBlock* next; 
};


struct ShaderTextureFunctionBlockTexture  // this is a member of d3d_commands_2 
{
	ShaderConstantFunction function;
	ShaderValueType valueType;
	u8 index;
	u8 parameter;
};

struct ShaderTextureFunctionBlock  // this is d3d_commands_2 
{
	u32 textureCount;
	ShaderTextureFunctionBlockTexture* textures;
	ShaderTextureFunctionBlock* next; 
};















