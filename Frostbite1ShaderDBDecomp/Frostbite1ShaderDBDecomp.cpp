#include "typedefs.h"
#include "DICEShaderSystem.h"
#include "memory_manager.h"
#include "fb_array.h"
#include "rb.h"
#include "rb_data.h"
#include "tinyxml2.h"
#include <map>
#include <shlwapi.h>
#include <iomanip>     
#include <filesystem>
#include <iostream>

#define RIGID_MESH 0
#define SKINNED_MESH 1 
#define COMPOSITE_MESH 2
#define TREE_MESH 4
#define GUIDSIZETEXT 36
#define SHADERCONSTANTDATASIZE 32
#define SOLUTIONSIZE 48
#define SOLUTIONSTATESIZE 88 
#define GEOMETRYDECLARATIONSIZE 89


void PrintGuid(u8* pGuid); 
void GuidToText(u8* pGuid, char* guid); 
i32 bc2_hasher(char* src); 
i32 bc2_hasher(char* src, u32 acc); 
void to_lower_inplace(char* src); 
void filepath_slash_swap(char* src);
void bundle_directory_test(char* dst_path);

void UpdateDatabaseFromRawFileV2(void* dbFileBuffer, std::map<u32, ShaderStruct*>* gShaderMap, rbtree* geometryDeclarationsMap, rbtree* gShadersGuidToNameMap);
void LinkShaderToSolutions(ShaderStruct* target, void* solutionsBlockStart, void* targetShaderRefBlock);
void LinkSolutionToData(ShaderSolution* solution, void* ShaderConstantBlockStart, void* VertexShadersBlockStart, void* PixelShaderBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart);


void GetMeshTokenFromMeshSet(const char* rsrcPath, const char* filePath); 

char* ShaderConstantBlockBypassOnly(void* dbFileBuffer);
ShaderConstants* ShaderConstantBlockGet(void* dbFileBuffer, i32 index);
char* ShaderFunctionBlockBypassOnly(void* dbFileBuffer);
ShaderConstantFunctionBlock* ShaderFunctionBlockGet(void* dbFileBuffer, u32 index);
ShaderTextureFunctionBlock* ShaderTextureFunctionBlockGet(void* dbFileBuffer, u32 index);
char* VertexShaderBlockBypassOnly(void* dbFileBuffer);
Dx11VertexShaderPermutation* VertexShaderBlockGet(void* dbFileBuffer, i32 index, void* ShaderConstantBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart);
char* PixelShaderBlockBypassOnly(void* dbFileBuffer); 
Dx11PixelShaderPermutation* PixelShaderBlockGet(void* dbFileBuffer, i32 index, void* ShaderConstantBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart);
char* SolutionsAndSolutionStateBlocksBypassOnly(void* dbFileBuffer); 
char* GeometryDeclarationBlockBypassOnly(void* dbFileBuffer);
char* GeometryDeclarationBlockPassAndUpdate(void* dbFileBuffer, rbtree* geometryDeclarationsMap); 

i32 XmlGetClassInstancesByName(tinyxml2::XMLDocument* doc, const char* targetName, tinyxml2::XMLElement** resClasses);
tinyxml2::XMLElement* XmlGetPrimaryInstance(tinyxml2::XMLDocument* doc); 
tinyxml2::XMLElement* XmlInstanceGetVarByName(tinyxml2::XMLElement* instance, const char* targetName);
tinyxml2::XMLElement* XmlFindInstance(tinyxml2::XMLElement* instance_runner, char* target);
const tinyxml2::XMLAttribute* XmlVarGetAttributeByName(tinyxml2::XMLElement* var, const char* targetName);

int ShaderSerializeXml(const char* savePath, ShaderStruct* shader, bool isOriginal);
void ShaderConstantSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderConstants* shaderConstants, const char* label); 
void Dx11VertexShaderPermutationSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, Dx11VertexShaderPermutation* permutation); 
void Dx11PixelShaderPermutationSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, Dx11PixelShaderPermutation* permutation); 
void ShaderFunctionConstantSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderConstantFunctionBlock* functionBlock);
void ShaderTextureFunctionSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderTextureFunctionBlock* functionBlock); 
void GeometryDeclarationSerializeXml(rbtree* geometryDeclMap); 
void VertexShadersSerializeToFiles(rbtree* Dx11VertexPermutationsMap); 
void PixelShadersSerializeToFiles(rbtree* Dx11PixelPermutationsMap); 
void FragmentShadersSerializeXml(std::map<u32, FragmentShaderStruct*>* gFragmentShadersMap); 



u32 modulePathLen = 0; 
char exePath[512] = { 0 };
const char* shadersPath = "./editor/shaders/"; 
const char* levelsPathDefault = "./editor/resources/levels/";
const char* resourcesPathDefault = "./editor/resources/";
char* gOutputPath = NULL;

// created by fuckass AI from the enums 
const char* ShaderGeometrySpaceNames[] =
{
	"SgsObject",
	"SgsWorld",
	"SgsScreen"
};

const char* ShaderSkinningMethodNames[] =
{
	"ShaderSkinningMethod_None",
	"ShaderSkinningMethod_1Bone",
	"ShaderSkinningMethod_2Bone",
	nullptr,
	"ShaderSkinningMethod_4Bone",
	nullptr,
	nullptr,
	nullptr,
	"ShaderSkinningMethod_8Bone",
	"ShaderSkinningMethod_Null"
};

const char* ShaderInstancingMethodNames[] =
{
	"ShaderInstancingMethod_None",
	"ShaderInstancingMethod_WorldTransform4x3Float",
	"ShaderInstancingMethod_WorldTransform4x3Half"
};

const char* ShaderRenderModeNames[] =
{
	"ShaderRenderMode_Normal",
	"ShaderRenderMode_Shadowbuffer",
	"ShaderRenderMode_ShadowbufferZTrans",
	"ShaderRenderMode_Shadowmap",
	"ShaderRenderMode_PointShadowbuffer",
	"ShaderRenderMode_ZOnly",
	"ShaderRenderMode_Velocity",
	"ShaderRenderMode_Simple",
	"ShaderRenderMode_SimpleLighting",
	"ShaderRenderMode_Overdraw"
};

const char* ShaderShadowmapMethodNames[] =
{
	"ShaderShadowmapMethod_None",
	"ShaderShadowmapMethod_ManualPcf",
	"ShaderShadowmapMethod_HardwarePcf",
	"ShaderShadowmapMethod_CascadedBox3",
	"ShaderShadowmapMethod_CascadedBox4",
	"ShaderShadowmapMethodCount"
};

const char* ShaderConstantFunctionNames[] =
{
	"ScfViewMatrix",
	"ScfViewMatrixTranspose",
	"ScfViewProjMatrix",
	"ScfViewProjMatrixTranspose",
	"ScfWorldViewMatrix",
	"ScfWorldViewMatrixTranspose",
	"ScfWorldViewProjMatrix",
	"ScfWorldViewProjMatrixTranspose",
	"ScfWorldMatrix",
	"ScfWorldMatrixTranspose",
	"ScfSpriteMatrix",
	"ScfSpriteMatrixTranspose",
	"ScfPrevViewMatrix",
	"ScfPrevViewMatrixTranspose",
	"ScfPrevViewProjMatrix",
	"ScfPrevViewProjMatrixTranspose",
	"ScfPrevWorldViewProjMatrix",
	"ScfPrevWorldViewProjMatrixTranspose",
	"ScfPrevWorldMatrix",
	"ScfPrevWorldMatrixTranspose",
	"ScfPrevSpriteMatrix",
	"ScfPrevSpriteMatrixTranspose",
	"ScfViewportZMinMax",
	"ScfBoneMatrices",
	"ScfCameraPos",
	"ScfTime",
	"ScfInvScreenSize",
	"ScfPointLightPos",
	"ScfPointLightColor",
	"ScfPointLightRadius",
	"ScfOutdoorLightDir",
	"ScfOutdoorLightHemisphereDir",
	"ScfOutdoorLightKeyColor",
	"ScfOutdoorLightKeySpecularColorAndHemisphereVisibility",
	"ScfOutdoorLightTopColor",
	"ScfOutdoorLightBottomColor",
	"ScfOutdoorLightShadowTransform",
	"ScfOutdoorLightShadowmapSizeAndInvSize",
	"ScfOutdoorLightShadowmapTexture",
	"ScfOutdoorLightReflectionEnvmap",
	"ScfOutdoorLightCloudShadowInvScaleAndIntensity",
	"ScfFogStartAndEnd",
	"ScfFogColorAndClamp",
	"ScfFogColorSunAndPower",
	"ScfFogHeightFalloffAndOffset",
	"ScfTransparentStartAndEndAndClamp",
	"ScfLightScatteringSkyBetaR",
	"ScfLightScatteringSkyBetaM",
	"ScfLightScatteringSunDirection",
	"ScfLightScatteringSunIrradianceAndEccentrity",
	"ScfLightScatteringEarthRadiusAndZenithHeightAndHazeHeight",
	"ScfLightScatteringFogDensityAndEccentrity",
	"ScfOffsetTexture",
	"ScfColorScale",
	"ScfDebugNonFiniteColor",
	"ScfXenonInstancingIndexCountAndStart",
	"ScfPs3ClipPlane",
	"ShaderConstantFunctionCount"
};

// these are not accurate 
const char* ShaderValueTypeNames[] =
{
	"SvtHalf1",
	"SvtHalf1x2",
	"SvtHalf1x3",
	"SvtHalf1x4",
	"SvtHalf2",
	"SvtHalf2x2",
	"SvtHalf2x3",
	"SvtHalf2x4",
	"SvtHalf3",
	"SvtHalf3x2",
	"SvtHalf3x3",
	"SvtHalf3x4",
	"SvtHalf4",
	"SvtHalf4x2",
	"SvtHalf4x3",
	"SvtHalf4x4",
	"SvtFloat1",
	"SvtFloat1x2",
	"SvtFloat1x3",
	"SvtFloat1x4",
	"SvtFloat2",
	"SvtFloat2x2",
	"SvtFloat2x3",
	"SvtFloat2x4",
	"SvtFloat3",
	"SvtFloat3x2",
	"SvtFloat3x3",
	"SvtFloat3x4",
	"SvtFloat4",
	"SvtFloat4x2",
	"SvtFloat4x3",
	"SvtFloat4x4",
	"SvtInt1",
	"SvtInt1x2",
	"SvtInt1x3",
	"SvtInt1x4",
	"SvtInt2",
	"SvtInt2x2",
	"SvtInt2x3",
	"SvtInt2x4",
	"SvtInt3",
	"SvtInt3x2",
	"SvtInt3x3",
	"SvtInt3x4",
	"SvtInt4",
	"SvtInt4x2",
	"SvtInt4x3",
	"SvtInt4x4",
	"SvtBool1",
	"SvtBool1x2",
	"SvtBool1x3",
	"SvtBool1x4",
	"SvtBool2",
	"SvtBool2x2",
	"SvtBool2x3",
	"SvtBool2x4",
	"SvtBool3",
	"SvtBool3x2",
	"SvtBool3x3",
	"SvtBool3x4",
	"SvtBool4",
	"SvtBool4x2",
	"SvtBool4x3",
	"SvtBool4x4",
	"SvtSampler2D",
	"SvtSampler3D",
	"SvtSamplerCube",
	"SvtCount"
};

const char* ShaderVertexElementUsageNames[] =
{
	"ShaderVertexElementUsage_Unknown",
	"ShaderVertexElementUsage_Pos",
	"ShaderVertexElementUsage_BoneIndices",
	"ShaderVertexElementUsage_BoneWeights",
	"ShaderVertexElementUsage_Normal",
	"ShaderVertexElementUsage_Tangent",
	"ShaderVertexElementUsage_Binormal",
	"ShaderVertexElementUsage_BinormalSign",
	"ShaderVertexElementUsage_WorldTrans1",
	"ShaderVertexElementUsage_WorldTrans2",
	"ShaderVertexElementUsage_WorldTrans3",
	"ShaderVertexElementUsage_XenonIndex"
};
//


rbtree* gVertexShaderPermutations = NULL; 
rbtree* gPixelShaderPermutations = NULL; 


std::vector<ShaderConstantFunctionBlock*> gShaderConstantFunctionBlocks; 
std::vector<ShaderTextureFunctionBlock*> gShaderTextureFunctionBlocks;
std::map<void*, ShaderConstants*> gShaderConstants;
std::map<void*, ShaderConstantFunctionBlock*> gShaderConstantFunctionMap;
std::map<void*, ShaderTextureFunctionBlock*> gShaderTextureFunctionMap;
std::map<u32, FragmentShaderStruct*> gFragmentShadersMap; 
// kept for crawlerV1 function only 
std::map<u32, char*> shadersGatheredFromResourcesMap;
// crawlerV2 uses this
std::map<u32, char*> meshTokensFromGameDump;

// for debugging
std::map<u32, char*> gMeshNodesMap; 
FILE* gFragmentShadersTxt = NULL;
FILE* gUnreferencedMeshNodesTxt = NULL;
FILE* gShadersNonShaderString = NULL; 
//

mm_manager* gMemoryManager = NULL;

/* decompiles shaders to a more readable format. 2 options. 
1) crawling game resources to get the original mesh names and get the proper shader name 
2) generating shaders as they come from the database. this means that the paths will be sometimes twice as long. for example: 
   no crawling final path: Objects/Weapons/Handheld/BU_rif_F2000/BU_rif_F2000_Mesh3p/Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal 
   path after resource crawling: Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal
   but aside from that the actual shader.xml files will be the same.
*/
int main()
{
	init_memory_manager();
    mm_manager* memoryManager = get_memory_manager(); 
	gMemoryManager = memoryManager; 
    mm_pool* mainPool = memoryManager->create_pool(MEGABYTE * 512); 
    mm_page* mainPage = memoryManager->reserve_pool(mainPool, MEMPAGE, MEGABYTE * 512); 
    mm_pool* tempPool = memoryManager->create_pool(MEGABYTE * 16);
    mm_page* tempPage = memoryManager->reserve_pool(tempPool, MEMPAGETEMP, MEGABYTE * 16);
	char levelsPath[512] = { 0 };
	char resourcesPath[512] = { 0 };
	char startupPath[512] = { 0 };
	char outputPath[512] = { 0 };

	std::map<u32, ShaderStruct*> gShaders;
	std::map<u32, ShaderStruct*>::iterator it;
	std::map<u32, char*>::iterator it2; 

	GetModuleFileNameA(NULL, exePath, sizeof(exePath) - 1);
	modulePathLen = strlen(exePath); 
	rbtree* geometryDeclarationsMap = rb_create(compare_func, destroy_func);
	rbtree* gShadersGuidToNameMap = rb_create(compare_func, destroy_func);
	gVertexShaderPermutations = rb_create(compare_func, destroy_func);
	gPixelShaderPermutations = rb_create(compare_func, destroy_func);
	// for debugging
	bundle_directory_test(const_cast<char*>("./debug/"));
	FILE* fragmentShadersTxt = fopen("./debug/fragmentShadersList.txt", "w"); 
	FILE* unreferencedMeshNodesTxt = fopen("./debug/unreferencedMeshNodes.txt", "w");
	FILE* shadersNonShaderString = fopen("./debug/shadersNonShaders.txt", "w");
	gFragmentShadersTxt = fragmentShadersTxt; 
	gUnreferencedMeshNodesTxt = unreferencedMeshNodesTxt;
	gShadersNonShaderString = shadersNonShaderString; 
	//
	bool crawlResources = FALSE; 
	std::cout << "DO YOU WANT TO CRAWL ALL RESOURCES FOR MORE ACCURATE DECOMPILATION? no = 0, yes = 1" << std::endl; 
	std::cin >> crawlResources; 
	if (crawlResources)
	{
		char temp[512] = { 0 };
		std::cout << "TYPE EDITOR FOLDER PATH WITH A SLASH AT THE END (/). EXAMPLE: C:\\your\\path\\" << std::endl; 
		std::cin >> std::setw(sizeof(temp) - 1) >> temp;
		filepath_slash_swap(temp);
		to_lower_inplace(temp); 
		snprintf(resourcesPath, sizeof(resourcesPath) - 1, "%s/", temp);
		snprintf(startupPath, sizeof(startupPath) - 1, "%sasync/", temp);
	}

	/*
	scan every file(fucking slow! beautiful)
	the trick is to mix the object name + the shader name. this gives the full token found in the shader database. hash this full token as if it were straight out of the 
	shader database, but keep the short original path instead. when creating the file itself take the hash from shader database, but the original path from this parallel map
	*/
	if (crawlResources)
	{
	
		std::string path = resourcesPath;
		std::filesystem::path current_path = resourcesPath;
		std::filesystem::recursive_directory_iterator iterator = std::filesystem::recursive_directory_iterator(path);
		for (const auto& entry : iterator)
		{
			if (std::filesystem::is_regular_file(entry.status()))
			{
				std::string s = entry.path().lexically_relative(current_path).string();
				char lowercaseFile[512] = { 0 };
				strncpy(lowercaseFile, s.c_str(), sizeof(lowercaseFile) - 1);
				filepath_slash_swap(lowercaseFile);
				to_lower_inplace(lowercaseFile);
				if (strstr(lowercaseFile, ".xml"))
				{
					char fullFilePath[512] = { 0 };
					snprintf(fullFilePath, sizeof(fullFilePath) - 1, "%s%s", path.c_str(), lowercaseFile);
					tinyxml2::XMLDocument tempdoc;
					if (tempdoc.LoadFile(fullFilePath) == tinyxml2::XML_SUCCESS)
					{
						tinyxml2::XMLElement* shaderSetInstances[64] = { 0 };
						tinyxml2::XMLElement* primaryInstance = XmlGetPrimaryInstance(&tempdoc);
						if (primaryInstance)
						{
							char lowercaseClassName[96] = { 0 };
							const char* className = primaryInstance->FirstAttribute()->Next()->Value();
							strncpy(lowercaseClassName, className, sizeof(lowercaseClassName) - 1);
							to_lower_inplace(lowercaseClassName);
							if (strstr(lowercaseClassName, "meshasset"))
							{
								std::map<u32, char*>::iterator it; 
								char lowercaseToken[256] = { 0 };
								char* realToken = (char*)memoryManager->allocate(MEMPAGE, sizeof(lowercaseToken)); 
								tinyxml2::XMLElement* baseName = XmlInstanceGetVarByName(primaryInstance, "name");					
								snprintf(lowercaseToken, sizeof(lowercaseToken) - 1, "%s/", baseName->GetText());
								snprintf(realToken, sizeof(lowercaseToken) - 1, "%s/", baseName->GetText());
								to_lower_inplace(lowercaseToken);
								u32 hash = bc2_hasher(lowercaseToken);
								it = meshTokensFromGameDump.find(hash);
								if (it == meshTokensFromGameDump.end())
								{
									std::cout << "MESH ASSET TOKEN FOUND: " << baseName->GetText() << std::endl;
									meshTokensFromGameDump.insert({ hash,realToken });
								}
							}
						}
					}
				}
				else if (strstr(lowercaseFile, "meshset"))
				{
					GetMeshTokenFromMeshSet(resourcesPath, lowercaseFile); 
				}
			}
		}
		std::cout << "MESH TOKENS GATHERED FROM RESOURCES: " << meshTokensFromGameDump.size() << std::endl;
		for (it2 = meshTokensFromGameDump.begin(); it2 != meshTokensFromGameDump.end(); it2++)
		{
			std::cout << it2->second << std::endl;
		}
		
	}
	
	if (PathFileExistsA("./editor/resources/levels/") == FALSE)
	{
		std::cout << "TYPE SHADER DATABASE PATH (WITH A FORWARD SLASH AT THE END) - THE PROGRAM WILL SEARCH FOR ANY SHADERDATABASE FILE IN THIS PATH. EXAMPLE: C:\\your\\path\\" << std::endl;
		std::cin >> std::setw(sizeof(levelsPath) - 1) >> levelsPath; 
	}
	else strncpy(levelsPath, levelsPathDefault, sizeof(levelsPath) - 1); 
	filepath_slash_swap(levelsPath);
	std::string path = levelsPath;
	std::filesystem::path current_path = levelsPath;
	std::filesystem::recursive_directory_iterator iterator = std::filesystem::recursive_directory_iterator(path);
	for (const auto& entry : iterator)
	{
		if (std::filesystem::is_regular_file(entry.status()))
		{
			std::string s = entry.path().lexically_relative(current_path).string();
			char lowercaseFile[512] = { 0 };
			strncpy(lowercaseFile, s.c_str(), sizeof(lowercaseFile) - 1);
			filepath_slash_swap(lowercaseFile); 
			to_lower_inplace(lowercaseFile); 
			if (strstr(lowercaseFile, ".dx11shaderdatabase"))
			{
				char fullfilePath[512] = { 0 }; 
				snprintf(fullfilePath, sizeof(fullfilePath) - 1, "%s%s", levelsPath, lowercaseFile); 
				std::cout << "SHADERDATABASE FOUND : " << fullfilePath << std::endl << std::endl;
				FILE* test = fopen(fullfilePath, "r+b");
				if (test == NULL) __debugbreak(); 
				fseek(test, 0, SEEK_END);
				i32 filesize = ftell(test);
				fseek(test, 0, SEEK_SET);
				void* buffer = malloc(filesize);
				fread(buffer, 1, filesize, test);
				fclose(test);
				UpdateDatabaseFromRawFileV2(buffer, &gShaders, geometryDeclarationsMap, gShadersGuidToNameMap);
				free(buffer);
    		}
		}
	}
	std::cout << "SHADER CONSTANT FUNCTION BLOCKS: " << gShaderConstantFunctionMap.size() << std::endl; 
	std::cout << "SHADER TEXTURE FUNCTION BLOCKS: " << gShaderTextureFunctionMap.size() << std::endl; 
	std::cout << "SHADER CONSTANTS BLOCKS: " << gShaderConstants.size() << std::endl;
	std::cout << "TOTAL SHADERS: " << gShaders.size() << std::endl;

	std::cout << "TYPE OUTPUT PATH WITH A SLASH AT THE END (/). EXAMPLE: C:\\your\\path\\. MUST BE ABSOLUTE PATH - NO ./ PREFIX " << std::endl;
	std::cin >> std::setw(sizeof(outputPath) - 1) >> outputPath;
	gOutputPath = outputPath;


	GeometryDeclarationSerializeXml(geometryDeclarationsMap);
	VertexShadersSerializeToFiles(gVertexShaderPermutations);
	PixelShadersSerializeToFiles(gPixelShaderPermutations); 
	FragmentShadersSerializeXml(&gFragmentShadersMap);
	std::cout << "TOTAL NUMBER OF SHADERS: " << gShaders.size() << std::endl;
	for (it = gShaders.begin(); it != gShaders.end(); it++)
	{
		char fullPath[512] = { 0 };
		ShaderStruct* shader = it->second;
		std::cout << "Shader: " << shader->name << std::endl;
		snprintf(fullPath, sizeof(fullPath) - 1, "%s%s.xml", gOutputPath, shader->name);
		bundle_directory_test(fullPath);
		ShaderSerializeXml(fullPath, shader, TRUE);
	}
	fclose(fragmentShadersTxt); 
	fclose(unreferencedMeshNodesTxt); 
	fclose(shadersNonShaderString); 
    return 0; 
}



void GetMeshTokenFromMeshSet(const char* rsrcPath, const char* filePath)
{
	static mm_manager* memoryManager = get_memory_manager(); 
	char fullPath[512] = { 0 };
	i8 meshType = -1; 
	if (strstr(filePath, ".compositemeshset")) meshType = COMPOSITE_MESH; 
	else if (strstr(filePath, ".rigidmeshset")) meshType = RIGID_MESH;
	else if (strstr(filePath, ".treemeshset")) meshType = TREE_MESH;
	else if (strstr(filePath, ".skinnedmeshset")) meshType = SKINNED_MESH;
	if (meshType == -1) return; 
	snprintf(fullPath, sizeof(fullPath) - 1, "%s%s", rsrcPath, filePath); 
	void* fileBuffer = malloc(MEGABYTE * 5);
	FILE* meshsetFile = fopen(fullPath, "r+b");
	if (meshsetFile == NULL) __debugbreak();
	fseek(meshsetFile, 0, SEEK_END);
	u32 filesize = ftell(meshsetFile);
	fseek(meshsetFile, 0, SEEK_SET);
	fread(fileBuffer, 1, filesize, meshsetFile);
	fclose(meshsetFile);
	bool carry = FALSE; 
	char* runner = (char*)fileBuffer;
	runner += 5;
	char* lod_settings_fp = runner;
	runner += strlen(runner) + 1;
	i32 n_lods = (u32)*runner;
	runner += sizeof(u32);
	i8 unknown = -1; 
	i8 n_shaders_per_set = -1; 
	i8 n_shader_sets = -1;
//	for (int i = 0; i < n_lods; i++)
	for (int i = 0; i < 1; i++)
	{
		unknown = *runner;
		n_shaders_per_set = *(runner + 1);
		n_shader_sets = *(runner + 2);
		runner += 3;
		for (int j = 0; j < n_shader_sets; j++) // shader set level. this is the same for all mesh sets
		{
			std::map<u32, char*>::iterator it; 
			char lowercaseMeshPath[384] = { 0 };
			char nameNoSlash[384] = { 0 };
			u32 meshpathlen = 0;
			if (*runner)
			{
				char* meshpath = (char*)memoryManager->allocate(MEMPAGE, sizeof(lowercaseMeshPath));
				char* meshToken = runner;
				char* slash = strstr(runner, "/");
				char* guid = slash++;
				while (slash)
				{
					slash++;
					guid = slash;
					slash = strstr(slash, "/");
				}
				meshpathlen = guid - runner;
				strncpy(meshpath, runner, meshpathlen);
				strncpy(nameNoSlash, runner, meshpathlen-1);
				strncpy(lowercaseMeshPath, meshpath, sizeof(lowercaseMeshPath) - 1);
				to_lower_inplace(lowercaseMeshPath);
				u32 hash = bc2_hasher(lowercaseMeshPath); 
				it = meshTokensFromGameDump.find(hash); 
				if (it == meshTokensFromGameDump.end())
				{
					std::cout << "MESH ASSET TOKEN FOUND: " << nameNoSlash << std::endl;
					meshTokensFromGameDump.insert({ hash,meshpath });
				}
				goto fuckOff;
				runner += strlen(runner) + 1;
			}
			else
			{
				carry = true;
				runner++;
			}
		}
	}
	fuckOff: 
	free(fileBuffer);
	return; 
}

void RBTreeGetPointers(rbtree* rbt, rbnode* n, std::vector<void*>* dst)
{
	if (n != RB_NIL(rbt)) {
		mydata_mm128* midman = (mydata_mm128*)n->data;
		dst->push_back(midman->data);
		RBTreeGetPointers(rbt, n->right, dst);
		RBTreeGetPointers(rbt, n->left, dst);
	}
}



const char* MeshTokenFromShaderToken(const char* shaderTokenLowercase)
{
	std::map<u32, char*>::iterator it;
	for (it = meshTokensFromGameDump.begin(); it != meshTokensFromGameDump.end(); it++)
	{
		char lowercaseToken[256] = { 0 };
		strncpy(lowercaseToken, it->second, sizeof(lowercaseToken) - 1);
		to_lower_inplace(lowercaseToken); 
		if (strstr(shaderTokenLowercase, lowercaseToken)) return it->second;
	}
	return NULL; 
}

const char* MeshTokenFromMeshNodeToken(const char* shaderTokenLowercase)
{
	std::map<u32, char*>::iterator it;
	for (it = meshTokensFromGameDump.begin(); it != meshTokensFromGameDump.end(); it++)
	{
		char lowercaseToken[256] = { 0 };
		strncpy(lowercaseToken, it->second, strlen(it->second)-1);
		to_lower_inplace(lowercaseToken);
		if (strstr(shaderTokenLowercase, lowercaseToken)) return it->second;
	}
	return NULL;
}




/* gShaderMap holds the shader struct with the real token. example: 
* full shader token from database: Objects/Weapons/Handheld/BU_rif_F2000/BU_rif_F2000_Mesh3p/Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal
  mesh token = Objects/Weapons/Handheld/BU_rif_F2000/BU_rif_F2000_Mesh3p <- this comes from crawl resources v2
  shader token (gShadersMap hash) = Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal <- hash this string after converting it to lowercase 
*/
void UpdateDatabaseFromRawFileV2(void* dbFileBuffer, std::map<u32, ShaderStruct*>* gShaderMap, rbtree* geometryDeclarationsMap, rbtree* gShadersGuidToNameMap)
{
	static mm_manager* memoryManager = get_memory_manager();
	char* fileBuffer = (char*)dbFileBuffer + 5;
	rbtree* shadersGuidToNameToBuild = rb_create(compare_func, destroy_func);
	char* shaderConstantsBlock = fileBuffer;
	char* shaderConstantFunctionBlock = ShaderConstantBlockBypassOnly(shaderConstantsBlock);
	char* shaderTextureFunctionBlock = ShaderFunctionBlockBypassOnly(shaderConstantFunctionBlock); // bypass constant function block
	char* vertexShadersBlock = ShaderFunctionBlockBypassOnly(shaderTextureFunctionBlock); // bypass texture function block (they have the same layout) 
	char* pixelShadersBlock = VertexShaderBlockBypassOnly(vertexShadersBlock);
	char* pSolutionsBlockStart = PixelShaderBlockBypassOnly(pixelShadersBlock);
	//	pSolutionsBlockStart = fileBuffer; 
	fileBuffer = SolutionsAndSolutionStateBlocksBypassOnly(pSolutionsBlockStart);
	fileBuffer = GeometryDeclarationBlockPassAndUpdate(fileBuffer, geometryDeclarationsMap);
	u32* nShaders = (u32*)fileBuffer;
	fileBuffer += 4;
	std::map<u32, ShaderStruct*>::iterator it;
	std::map<u32, FragmentShaderStruct*>::iterator it2; 
	std::map<u32, char*>::iterator it3;

	for (int i = 0; i < *nShaders; i++)
	{
		char lowercaseName[512] = { 0 };
		char resultName[sizeof(lowercaseName)] = { 0 }; 
		char resultNameOriginal[sizeof(lowercaseName)] = { 0 }; 
		strncpy(lowercaseName, fileBuffer, sizeof(lowercaseName) - 1);
		to_lower_inplace(lowercaseName);
		const char* isCompoundPath = MeshTokenFromShaderToken(lowercaseName); 
		i32 meshTokenLen = 0; 
		if (isCompoundPath) meshTokenLen = strlen(isCompoundPath);
		snprintf(resultName, sizeof(resultName) - 1, "%s", fileBuffer + meshTokenLen);
		memcpy(resultNameOriginal, resultName, sizeof(lowercaseName));
		to_lower_inplace(resultName);
		u32 hash = bc2_hasher(resultName);
		it = gShaderMap->find(hash);
		if (it == gShaderMap->end())
		{
			// debugging 
			if (isCompoundPath == NULL)
			{
				fprintf(gShadersNonShaderString, "%s\n", resultNameOriginal);
			}
			ShaderStruct* shader = (ShaderStruct*)memoryManager->allocate(MEMPAGE, sizeof(ShaderStruct));
			shader->name = (char*)memoryManager->allocate(MEMPAGE, sizeof(lowercaseName));
			strncpy(shader->name, resultNameOriginal, sizeof(lowercaseName));
			gShaderMap->insert({ hash,shader });
			char* pGuid = fileBuffer + strlen(fileBuffer) + 1;
			memcpy(shader->id, pGuid, 16);
			__m128i guid = _mm_loadu_epi8(pGuid);
			// for the temp map 
			mydata_mm128* p = (mydata_mm128*)allocate_aligned(MEMPAGETEMP, sizeof(mydata_mm128), 16);
			p->key = guid;
			p->data = (void*)shader->name;
			rb_insert(shadersGuidToNameToBuild, p);
			// for the global guid to name map 
			p = (mydata_mm128*)allocate_aligned(MEMPAGE, sizeof(mydata_mm128), 16);
			p->key = guid;
			p->data = (void*)shader->name;
			rb_insert(gShadersGuidToNameMap, p);
		}
		fileBuffer += strlen(fileBuffer) + 1;
		fileBuffer += 16;
	}
	// fuck this part for now
	u32* nFragmentShaders = (u32*)fileBuffer;
	fileBuffer += 4;
	for (int i = 0; i < *nFragmentShaders; i++)
	{
		char lowercaseName[256] = { 0 };
		strncpy(lowercaseName, fileBuffer, sizeof(lowercaseName) - 1);
		to_lower_inplace(lowercaseName);
		it2 = gFragmentShadersMap.find(bc2_hasher(lowercaseName));
		if (it2 == gFragmentShadersMap.end())
		{	
			FragmentShaderStruct* fragment = (FragmentShaderStruct*)memoryManager->allocate(MEMPAGE, sizeof(FragmentShaderStruct));
			fragment->name = (char*)memoryManager->allocate(MEMPAGE, sizeof(lowercaseName));
			strncpy(fragment->name, fileBuffer, sizeof(lowercaseName) - 1);
			char* pGuid = fileBuffer + strlen(fileBuffer) + 1;
			fb_array_init(&fragment->availableIds, MEMPAGE, 24, 16);
			fb_array_add(&fragment->availableIds, (void*)pGuid);
			gFragmentShadersMap.insert({ bc2_hasher(lowercaseName), fragment });
			if (gFragmentShadersTxt) fprintf(gFragmentShadersTxt, "%s\n", fragment->name);
		}
		else
		{
			FragmentShaderStruct* fragment = (FragmentShaderStruct*)it2->second; 
			char* pGuid = fileBuffer + strlen(fileBuffer) + 1;	
			for (int j = 0; j < fb_array_size(&fragment->availableIds); j++)
			{
				u8* pGuidToCompare = (u8*)fb_array_get(&fragment->availableIds, j); 
				if (memcmp(pGuid, pGuidToCompare, 16) != 0)
				{
					fb_array_add(&fragment->availableIds, (void*)pGuid);
				}
			}
		}
		fileBuffer += strlen(fileBuffer) + 1;
		fileBuffer += 16;
	}
	u32* nMeshNodes = (u32*)fileBuffer;
	fileBuffer += 4;
	for (int i = 0; i < *nMeshNodes; i++)
	{

		/* THIS IS JUST FOR DEBUGGING. ENABLING THIS DUMP WILL MAKE THE SCRIPT TAKE 7x LONGER. AND ITS REALLY UNNECESSARY
		char lowercaseName[512] = { 0 };
		char resultName[sizeof(lowercaseName)] = { 0 };
		char resultNameOriginal[sizeof(lowercaseName)] = { 0 };
		strncpy(lowercaseName, fileBuffer, sizeof(lowercaseName) - 1);
		to_lower_inplace(lowercaseName);
		const char* isCompoundPath = MeshTokenFromMeshNodeToken(lowercaseName);
		i32 meshTokenLen = 0;
		i32 offset = 0; 
		if (isCompoundPath)
		{
			meshTokenLen = strlen(isCompoundPath);
			offset = 5;
		}
		snprintf(resultName, sizeof(resultName) - 1, "%s", fileBuffer + meshTokenLen + offset);
		memcpy(resultNameOriginal, resultName, sizeof(lowercaseName));
		to_lower_inplace(resultName);
		u32 hash = bc2_hasher(resultName);
		it3 = gMeshNodesMap.find(hash);
		if (it3 == gMeshNodesMap.end())
		{
			gMeshNodesMap.insert({ hash,resultNameOriginal });
			if (isCompoundPath == NULL) fprintf(gUnreferencedMeshNodesTxt, "%s\n", resultNameOriginal); 
		}*/

		fileBuffer += strlen(fileBuffer) + 1;
		fileBuffer += 16;
	}
	u32* nSolutionsRefs = (u32*)fileBuffer;
	fileBuffer += 4;
	for (int i = 0; i < *nSolutionsRefs; i++)
	{
		__m128i guid = _mm_loadu_epi8(fileBuffer);
		mydata_mm128 p;
		p.key = guid;
		p.data = NULL;
		rbnode* n = rb_find(shadersGuidToNameToBuild, &p);
		if (n)
		{
			mydata_mm128* midman = (mydata_mm128*)n->data;
			char* shaderName = (char*)midman->data;
			char lowercaseName[512] = { 0 };
			strncpy(lowercaseName, shaderName, sizeof(lowercaseName) - 1);
			to_lower_inplace(lowercaseName);
			it = gShaderMap->find(bc2_hasher(lowercaseName));
			if (it != gShaderMap->end())
			{
				LinkShaderToSolutions(it->second, pSolutionsBlockStart, fileBuffer);
				for (int j = 0; j < it->second->nSolutions; j++)
				{

					ShaderSolution* solution = (ShaderSolution*)fb_array_get(&it->second->solutions, j);
					LinkSolutionToData(solution, shaderConstantsBlock, vertexShadersBlock, pixelShadersBlock, shaderConstantFunctionBlock, shaderTextureFunctionBlock);
				}
			}
			else __debugbreak();
		}
		fileBuffer += 16;
		u32* nRefs = (u32*)fileBuffer;
		fileBuffer += 4 + (*nRefs * 2);
	}

	rb_destroy(shadersGuidToNameToBuild);
}


void LinkSolutionToData(ShaderSolution* solution,void* ShaderConstantBlockStart, void* VertexShadersBlockStart, 
	void* PixelShaderBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart)
{
	
	solution->vertexShader = VertexShaderBlockGet(VertexShadersBlockStart, solution->vsIndex, ShaderConstantBlockStart, ShaderConstantFunctionBlockStart, ShaderTextureFunctionBlockStart);
	solution->pixelShader = PixelShaderBlockGet(PixelShaderBlockStart,solution->psIndex, ShaderConstantBlockStart, ShaderConstantFunctionBlockStart, ShaderTextureFunctionBlockStart);
	solution->vertexShaderResourceOverride = ShaderConstantBlockGet(ShaderConstantBlockStart, solution->vertexShaderResourceIndex);
	solution->pixelShaderResourceOverride = ShaderConstantBlockGet(ShaderConstantBlockStart, solution->pixelShaderResourceIndex);

	return; 
}

void LinkShaderToSolutions(ShaderStruct* target, void* solutionsBlockStart, void* targetShaderRefBlock)
{
	mm_manager* memoryManager = get_memory_manager(); 
	u32* nSolutions = (u32*)solutionsBlockStart;
	char* refBlockReader = (char*)targetShaderRefBlock;
	char* solutionsBlockData = (char*)solutionsBlockStart + sizeof(u32);
	char* solutionsStateBlockData = solutionsBlockData + (*nSolutions * SOLUTIONSIZE) + sizeof(u32);
	target->nSolutions = *(u32*)(refBlockReader + 16);
	fb_array_init(&target->solutions, MEMPAGE, target->nSolutions, sizeof(ShaderSolution));
	refBlockReader += 20; 
	u16* solutionIndex = (u16*)refBlockReader;
	for (int i = 0; i < target->nSolutions; i++)
	{	
		void* solution = solutionsBlockData + (*solutionIndex * SOLUTIONSIZE); 
		ShaderSolution solutionToAdd; 
		ShaderSolution* solutionCast = (ShaderSolution*)solution; 
		memcpy(solutionToAdd.id, solution, 16); 
		solutionToAdd.flags = solutionCast->flags;
		solutionToAdd.alphaRef = solutionCast->alphaRef; 
		solutionToAdd.hash = solutionCast->hash;
		solutionToAdd.null = solutionCast->null;
		solutionToAdd.vsIndex = solutionCast->vsIndex;
		solutionToAdd.psIndex = solutionCast->psIndex;
		solutionToAdd.vertexShaderResourceIndex = solutionCast->vertexShaderResourceIndex;
		solutionToAdd.pixelShaderResourceIndex = solutionCast->pixelShaderResourceIndex;
		solutionToAdd.solutionState = (ShaderSolutionState*)memoryManager->allocate(MEMPAGE, sizeof(ShaderSolutionState)); 
		memcpy(solutionToAdd.solutionState, solutionsStateBlockData + (*solutionIndex * SOLUTIONSTATESIZE), sizeof(ShaderSolutionState)); 
		fb_array_add(&target->solutions, &solutionToAdd);
		solutionIndex++; 
	}
	return; 
}


char* ShaderConstantBlockBypassOnly(void* dbFileBuffer)
{
	u32* iterations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < *iterations; i++)
	{
		u32* advancePointer = (u32*)res; 
		res += *advancePointer;
	}
	return res; 
}


ShaderConstants* ShaderConstantStorageCompare(void* shaderConstantElement)
{
	void* res = NULL; 
	std::map<void*, ShaderConstants*>::iterator it;
	u32* sizeToCompare = (u32*)shaderConstantElement; 
	for (it = gShaderConstants.begin(); it != gShaderConstants.end(); it++)
	{
		u32* size = (u32*)it->first; 
		if (*size == *sizeToCompare)
		{
			if (memcmp(shaderConstantElement, it->first, *size) == 0) return it->second;
		}
	}
	return NULL; 
}

ShaderConstants* ShaderConstantBlockGet(void* dbFileBuffer, i32 index)
{
	static mm_manager* memoryManager = get_memory_manager();
	u32* iterations = (u32*)dbFileBuffer;
	if (index > *iterations) return NULL;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < index; i++)
	{
		u32* advancePointer = (u32*)res;
		res += *advancePointer;
	}
	ShaderConstants* existsOrNotNigga = ShaderConstantStorageCompare(res); 
	if (existsOrNotNigga != NULL) return existsOrNotNigga;
	else
	{
		ShaderConstants* block = (ShaderConstants*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstants));
		u32 fuckSize = *((u32*)res); 
		void* shaderContantBlock = memoryManager->allocate(MEMPAGE, fuckSize);
		memcpy(shaderContantBlock, res, fuckSize);
		gShaderConstants.insert({ shaderContantBlock,block });
		memcpy(&block->totalSize, res, SHADERCONSTANTDATASIZE);

		res += SHADERCONSTANTDATASIZE;
		if (block->m_textureConstantCount > 0)
		{
			block->textureConstants = (ShaderConstantsTextureConstant*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantsTextureConstant) * block->m_textureConstantCount);
			memcpy(block->textureConstants, res, sizeof(ShaderConstantsTextureConstant) * block->m_textureConstantCount);
			res += sizeof(ShaderConstantsTextureConstant) * block->m_textureConstantCount; 
		}
		if (block->m_externalValueConstantCount > 0)
		{
			block->externalValueConstants = (ShaderConstantsExternalValueConstant*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantsExternalValueConstant) * block->m_externalValueConstantCount);
			memcpy(block->externalValueConstants, res, sizeof(ShaderConstantsExternalValueConstant) * block->m_externalValueConstantCount);
			res += sizeof(ShaderConstantsExternalValueConstant) * block->m_externalValueConstantCount;
		}
		if (block->m_externalTextureConstantCount > 0)
		{
			block->externalTextureConstants = (ShaderConstantsExternalTextureConstant*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantsExternalTextureConstant) * block->m_externalTextureConstantCount);
			memcpy(block->externalTextureConstants, res, sizeof(ShaderConstantsExternalTextureConstant) * block->m_externalTextureConstantCount);
			res += sizeof(ShaderConstantsExternalTextureConstant) * block->m_externalTextureConstantCount;
		}
		if (block->m_samplerStateCount > 0)
		{
			block->samplerStates = (ShaderConstantsSamplerState*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantsSamplerState) * block->m_samplerStateCount);
			memcpy(block->samplerStates, res, sizeof(ShaderConstantsSamplerState) * block->m_samplerStateCount);
			res += sizeof(ShaderConstantsSamplerState) * block->m_samplerStateCount;
		}		
		return block;
	}	
}

char* ShaderFunctionBlockBypassOnly(void* dbFileBuffer)
{
	u32* iterations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < *iterations; i++)
	{
		u16* advancePointer = (u16*)res;
		res += (*advancePointer * 4) + 4;
	}
	return res;
}


ShaderTextureFunctionBlock* ShaderTextureFunctionBlockCompareStorage(void* element)
{
	void* res = NULL;
	std::map<void*, ShaderTextureFunctionBlock*>::iterator it;
	for (it = gShaderTextureFunctionMap.begin(); it != gShaderTextureFunctionMap.end(); it++)
	{
		u16* pSizeBase = (u16*)it->first;
		u32 size = (*pSizeBase * 4) + 4;
		if (memcmp(element, it->first, size) == 0) return it->second;
	}
	return NULL;
}

ShaderTextureFunctionBlock* ShaderTextureFunctionBlockGet(void* dbFileBuffer, u32 index)
{
	static mm_manager* memoryManager = get_memory_manager();
	ShaderTextureFunctionBlock* block = NULL;
	u32* iterations = (u32*)dbFileBuffer;
	if (index > *iterations);
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < index; i++)
	{
		u16* advancePointer = (u16*)res;
		res += (*advancePointer * 4) + 4;
	}
	ShaderTextureFunctionBlock* exists = ShaderTextureFunctionBlockCompareStorage(res);
	if (exists != NULL) return exists; 
	else
	{
		block = (ShaderTextureFunctionBlock*)memoryManager->allocate(MEMPAGE, sizeof(ShaderTextureFunctionBlock));
		u16* pSizeBase = (u16*)res;
		u32 size = (*pSizeBase * 4) + 4;
		void* element = memoryManager->allocate(MEMPAGE, size);
		memcpy(element, res, size);
		gShaderTextureFunctionMap.insert({ element,block }); 
		block->textureCount = *((u32*)res);
		res += sizeof(u32);
		if (block->textureCount > 0)
		{
			block->textures = (ShaderTextureFunctionBlockTexture*)memoryManager->allocate(MEMPAGE, sizeof(ShaderTextureFunctionBlockTexture) * block->textureCount);
			memcpy(block->textures, res, sizeof(ShaderTextureFunctionBlockTexture) * block->textureCount);
		}
		return block;
	}
}


ShaderConstantFunctionBlock* ShaderConstantFunctionBlockCompareStorage(void* element)
{
	void* res = NULL;
	std::map<void*, ShaderConstantFunctionBlock*>::iterator it;
	u16* pSizeBaseToCmp = (u16*)element;
	u32 sizeToCmp = (*pSizeBaseToCmp * 4) + 4;
	for (it = gShaderConstantFunctionMap.begin(); it != gShaderConstantFunctionMap.end(); it++)
	{
		u16* pSizeBase = (u16*)it->first;
		u32 size = (*pSizeBase * 4) + 4;
		if (size == sizeToCmp)
		{
			if (memcmp(element, it->first, size) == 0) return it->second;
		}
	}
	return NULL;
}

ShaderConstantFunctionBlock* ShaderFunctionBlockGet(void* dbFileBuffer, u32 index)
{
	static mm_manager* memoryManager = get_memory_manager();
	u32* iterations = (u32*)dbFileBuffer;
	if (index > *iterations) __debugbreak(); 
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < index; i++)
	{
		u16* advancePointer = (u16*)res;
		res += (*advancePointer * 4) + 4;
	}

	ShaderConstantFunctionBlock* exists = ShaderConstantFunctionBlockCompareStorage(res);
	if (exists != NULL)
	{
		return exists;
	}
	else
	{
		u16* pSizeBase = (u16*)res;
		u32 size = (*pSizeBase * 4) + 4;
		void* element = memoryManager->allocate(MEMPAGE, size);
		memcpy(element, res, size);
		ShaderConstantFunctionBlock* block = (ShaderConstantFunctionBlock*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantFunctionBlock));
		gShaderConstantFunctionMap.insert({ element,block });
		block->constantCount = *((u16*)res);
		res += sizeof(u16);
		block->registerCount = *((u16*)res);
		res += sizeof(u16);
		block->constants = (ShaderConstantFunctionBlockConstant*)memoryManager->allocate(MEMPAGE, sizeof(ShaderConstantFunctionBlockConstant) * block->constantCount);
		memcpy(block->constants, res, sizeof(ShaderConstantFunctionBlockConstant) * block->constantCount);
		return block;
	}
}



char* VertexShaderBlockBypassOnly(void* dbFileBuffer)
{
	u32* iterations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < *iterations; i++)
	{
		res += 16; // skip the guid 
		u32* advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		res += sizeof(u32) * 3;
		advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		advancePointer = (u32*)res;
		res+= ((*advancePointer * 7) * 4) + (*advancePointer * 9) + 8;
	}
	return res;
}


Dx11PixelShaderPermutation* PixelShaderBlockGet(void* dbFileBuffer, i32 index, void* ShaderConstantBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart)
{
	static mm_manager* memoryManager = get_memory_manager();
	u32* iterations = (u32*)dbFileBuffer;
	if (index > *iterations) return NULL;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < index; i++)
	{
		res += 16; // skip the guid 
		u32* advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		res += sizeof(u32) * 3;
	}
	__m128i guid = _mm_loadu_epi8(res);
	mydata_mm128 p;
	p.key = guid;
	p.data = NULL;
	rbnode* n = rb_find(gPixelShaderPermutations, &p);
	if (n != NULL)
	{
		mydata_mm128* midman = (mydata_mm128*)n->data;
		return (Dx11PixelShaderPermutation*)midman->data;
	}
	else
	{
		Dx11PixelShaderPermutation* shader = (Dx11PixelShaderPermutation*)memoryManager->allocate(MEMPAGE, sizeof(Dx11PixelShaderPermutation));
		mydata_mm128* p = (mydata_mm128*)allocate_aligned(MEMPAGE, sizeof(mydata_mm128), 16);
		p->key = guid;
		p->data = shader;
		rb_insert(gPixelShaderPermutations, p);
		memcpy(shader->id, res, 16);
		res += 16;
		u32* pU32 = (u32*)res;
		shader->shaderBytecodeSize = *pU32;
		res += sizeof(u32);
		shader->shaderBytecode = memoryManager->allocate(MEMPAGE, shader->shaderBytecodeSize);
		memcpy(shader->shaderBytecode, res, shader->shaderBytecodeSize);
		res += shader->shaderBytecodeSize;
		shader->constants = ShaderConstantBlockGet(ShaderConstantBlockStart, *((u32*)res));
		shader->ScIndex = *((u32*)res);
		res += sizeof(u32);
		shader->constantFunctionBlock = ShaderFunctionBlockGet(ShaderConstantFunctionBlockStart, *((u32*)res));
		shader->ScfIndex = *((u32*)res);
		res += sizeof(u32);
		shader->textureFunctionBlock = ShaderTextureFunctionBlockGet(ShaderTextureFunctionBlockStart, *((u32*)res));
		shader->StfIndex = *((u32*)res);
		return shader;
	}
}

Dx11VertexShaderPermutation* VertexShaderBlockGet(void* dbFileBuffer, i32 index, void* ShaderConstantBlockStart, void* ShaderConstantFunctionBlockStart, void* ShaderTextureFunctionBlockStart)
{
	static mm_manager* memoryManager = get_memory_manager(); 
	Dx11VertexShaderPermutation* shader = NULL; 
	u32* iterations = (u32*)dbFileBuffer;
	if (index > *iterations) return NULL;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < index; i++)
	{    
		res += 16; 
		u32* advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		res += sizeof(u32) * 3;
		advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		advancePointer = (u32*)res;
		res += ((*advancePointer * 7) * 4) + (*advancePointer * 9) + 8;
	}
	__m128i guid = _mm_loadu_epi8(res);
	mydata_mm128 p;
	p.key = guid;
	p.data = NULL;
	rbnode* n = rb_find(gVertexShaderPermutations, &p); 
	if (n != NULL)
	{
		mydata_mm128* midman = (mydata_mm128*)n->data;
		return (Dx11VertexShaderPermutation*)midman->data;
	}
	else
	{
		shader = (Dx11VertexShaderPermutation*)memoryManager->allocate(MEMPAGE, sizeof(Dx11VertexShaderPermutation));
		mydata_mm128* p = (mydata_mm128*)allocate_aligned(MEMPAGE, sizeof(mydata_mm128), 16);
		p->key = guid;
		p->data = shader;		
		rb_insert(gVertexShaderPermutations, p);
		memcpy(shader->id, res, 16);
		res += 16;
		u32* pU32 = (u32*)res;
		shader->shaderBytecodeSize = *pU32;
		res += sizeof(u32);
		shader->shaderBytecode = memoryManager->allocate(MEMPAGE, shader->shaderBytecodeSize);
		memcpy(shader->shaderBytecode, res, shader->shaderBytecodeSize); 
		res += shader->shaderBytecodeSize;
		shader->constants = ShaderConstantBlockGet(ShaderConstantBlockStart, *((u32*)res));
		shader->ScIndex = *((u32*)res);
		res += sizeof(u32);
		shader->constantFunctionBlock = ShaderFunctionBlockGet(ShaderConstantFunctionBlockStart, *((u32*)res));
		shader->ScfIndex = *((u32*)res);
		res += sizeof(u32);
		shader->textureFunctionBlock = ShaderTextureFunctionBlockGet(ShaderTextureFunctionBlockStart, *((u32*)res));
		shader->StfIndex = *((u32*)res);
		res += sizeof(u32);
		shader->shaderBytecodeInputSignatureSize = *((u32*)res);
		res += sizeof(u32);
		shader->shaderBytecodeInputSignature = memoryManager->allocate(MEMPAGE, shader->shaderBytecodeInputSignatureSize);
		memcpy(shader->shaderBytecodeInputSignature, res, shader->shaderBytecodeInputSignatureSize);
		res += shader->shaderBytecodeInputSignatureSize;
		shader->nInputLayoutElements = *((u32*)res);
		res += sizeof(u32);
		shader->inputLayoutElements = (D3D11_INPUT_ELEMENT_DESC*)memoryManager->allocate(MEMPAGE, sizeof(D3D11_INPUT_ELEMENT_DESC) * shader->nInputLayoutElements);
		memcpy(shader->inputLayoutElements, res, sizeof(D3D11_INPUT_ELEMENT_DESC) * shader->nInputLayoutElements);
		res += sizeof(D3D11_INPUT_ELEMENT_DESC) * shader->nInputLayoutElements;
		res += sizeof(u32); // there is an extra u32 which is the number of semantic strings, it is the same as shader->nInputLayoutElements
		for (int i = 0; i < shader->nInputLayoutElements; i++)
		{

			shader->inputLayoutElements[i].SemanticName = (LPCSTR)memoryManager->allocate(MEMPAGE, strlen(res) + 2);
			strncpy((char*)shader->inputLayoutElements[i].SemanticName, res, strlen(res) + 1);
			res += strlen(res) + 1;
		}
		return shader;
	}
}


char* PixelShaderBlockBypassOnly(void* dbFileBuffer)
{
	u32* iterations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < *iterations; i++)
	{
		res += 16; // skip the guid 
		u32* advancePointer = (u32*)res;
		res += sizeof(u32);
		res += *advancePointer;
		res += sizeof(u32) * 3;
	}
	return res;
}

char* SolutionsAndSolutionStateBlocksBypassOnly(void* dbFileBuffer)
{
	u32* nSolutions = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	return res + 4 + (*nSolutions* SOLUTIONSIZE)+(*nSolutions * sizeof(ShaderSolutionState)); // extra 4 bytes cuz the second block also has a nSolutions u32 field 
}


char* GeometryDeclarationBlockPassAndUpdate(void* dbFileBuffer, rbtree* geometryDeclarationsMap)
{
	u32* nDeclarations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	for (int i = 0; i < *nDeclarations; i++)
	{
		__m128i id = _mm_loadu_epi8(res); 
		mydata_mm128 p;
		p.key = id;
		rbnode* n = rb_find(geometryDeclarationsMap, &p);
		if (n == NULL)
		{
			mydata_mm128* p = (mydata_mm128*)allocate_aligned(MEMPAGE, sizeof(mydata_mm128), 16);
			p->key = id;
			p->data = allocate(MEMPAGE, GEOMETRYDECLARATIONSIZE);
			memcpy(p->data, res, GEOMETRYDECLARATIONSIZE);
			rb_insert(geometryDeclarationsMap, p);
		}
		res += 89; 
	}
	return res; 
}


char* GeometryDeclarationBlockBypassOnly(void* dbFileBuffer)
{
	u32* nDeclarations = (u32*)dbFileBuffer;
	char* res = (char*)dbFileBuffer + 4;
	return res + (*nDeclarations * GEOMETRYDECLARATIONSIZE);
}


void to_lower_inplace(char* src)
{
	for (int i = 0; src[i]; i++)
	{
		src[i] = tolower(src[i]);
	}
	return;
}

i32 bc2_hasher(char* src)
{
	uint32_t accumulator = 0;
	uint32_t temp;
	uint32_t charwalker;
	uint32_t tempchar;
	charwalker = *src;
	while (charwalker != 0)
	{
		tempchar = charwalker - 65;
		if (tempchar < 25) {
			charwalker += 32;
		}
		temp = accumulator << 5;
		accumulator += temp;
		accumulator = accumulator ^ charwalker;
		src++;
		charwalker = *src;
	}
	return accumulator;
}


i32 bc2_hasher(char* src, u32 acc)
{
	uint32_t accumulator = 0;
	uint32_t temp;
	uint32_t charwalker;
	uint32_t tempchar;
	charwalker = *src;
	accumulator = acc;
	while (charwalker != 0)
	{
		tempchar = charwalker - 65;
		if (tempchar < 25) {
			charwalker += 32;
		}
		temp = accumulator << 5;
		accumulator += temp;
		accumulator = accumulator ^ charwalker;
		src++;
		charwalker = *src;
	}
	return accumulator;
}


void filepath_slash_swap(char* src)
{
	char* backslash = strstr(src, "\\");
	while (backslash)
	{
		*backslash = '/';
		backslash++;
		backslash = strstr(backslash, "\\");
	}
}

void filepath_slash_swap_to_backslash(char* src)
{
	char* backslash = strstr(src, "/");
	while (backslash)
	{
		*backslash = '\\';
		backslash++;
		backslash = strstr(backslash, "/");
	}
}

void PrintGuid(u8* pGuid)
{
	char guid[36] = { 0 };
	for (int i = 0; i < 16; i++)
	{
		sprintf(guid + (i * 2), "%02X", pGuid[i]);
	}
	std::cout << guid; 
	return; 
}

void GuidToText(u8* pGuid, char* guid)
{	
	for (int i = 0; i < 16; i++)
	{
		sprintf(guid + (i * 2), "%02X", pGuid[i]);
	}
	return;
}


void bundle_directory_test(char* dst_path)
{
	u32 slashcount = 0;
	char* directories[32] = { 0 };
	char* slash = strstr((dst_path + 2), "/");
	char temp[512] = { 0 };
	while (slash)
	{
		slash++;
		directories[slashcount] = slash;
		slashcount++;
		slash = strstr(slash, "/");
	}
	for (int i = 0; i < slashcount; i++)
	{
		strncpy(temp, dst_path, (directories[i] - dst_path));
		i32 fuck = strlen(temp);
		if (fuck + 2 + modulePathLen >= MAX_PATH)
		{
			snprintf(temp, sizeof(temp) - 1, "\\\\?\\"); 
			strncpy(temp + strlen(temp), dst_path, (directories[i] - dst_path));
			filepath_slash_swap_to_backslash(temp);
		}
		if (!PathFileExistsA(temp))
		{
			bool result = CreateDirectoryA(temp, NULL);
			if (result == FALSE) 
			{
				u32 error = GetLastError();
		//		__debugbreak();
			}
		}
	}
	return;
}

// look at this repetitive boilerplate lazy garbage lmfao 
int ShaderSerializeXml(const char* savePath, ShaderStruct* shader, bool isOriginal)
{
	char fixedPath[1024] = { 0 };
	snprintf(fixedPath, sizeof(fixedPath) - 1, "\\\\?\\%s", savePath);
	filepath_slash_swap_to_backslash(fixedPath); 
	char shaderGuid[GUIDSIZETEXT + 2] = { 0 };
	tinyxml2::XMLDocument tempDoc;
	tinyxml2::XMLDeclaration* decl = tempDoc.NewDeclaration("xml version=\"1.0\"");
	tinyxml2::XMLElement* shaderLevel = tempDoc.NewElement("Shader");
	shaderLevel->SetAttribute("isOriginalShader", isOriginal);
	tinyxml2::XMLElement* varLevel = tempDoc.NewElement("var");
	tinyxml2::XMLElement* solutionLevel = NULL;
	varLevel->SetAttribute("name", "Name");
	varLevel->SetText(shader->name);
	shaderLevel->InsertEndChild(varLevel);

	varLevel = tempDoc.NewElement("var");
	varLevel->SetAttribute("name", "Id");
	GuidToText((u8*)shader->id, shaderGuid);
	varLevel->SetText(shaderGuid);
	shaderLevel->InsertEndChild(varLevel);

	varLevel = tempDoc.NewElement("var");
	varLevel->SetAttribute("name", "nSolutions");
	varLevel->SetText(fb_array_size(&shader->solutions));
	shaderLevel->InsertEndChild(varLevel);

	varLevel = tempDoc.NewElement("var");
	varLevel->SetAttribute("name", "Solutions");

	for (int i = 0; i < fb_array_size(&shader->solutions); i++)
	{
		char solutionGuid[GUIDSIZETEXT + 2] = { 0 };
		char onlyGuid[GUIDSIZETEXT + 2] = { 0 };
		tinyxml2::XMLElement* solutionVarLevel = tempDoc.NewElement("var");
		tinyxml2::XMLElement* solutionStateLevel = tempDoc.NewElement("SolutionState");
		tinyxml2::XMLElement* solutionStateVarLevel = tempDoc.NewElement("var");

		solutionVarLevel->SetAttribute("name", "Id");
		ShaderSolution solution = *(ShaderSolution*)fb_array_get(&shader->solutions, i);
		GuidToText((u8*)solution.id, solutionGuid);
		solutionVarLevel->SetText(solutionGuid);
		solutionLevel = tempDoc.NewElement("ShaderSolution");
		solutionLevel->InsertEndChild(solutionVarLevel);

		solutionVarLevel = tempDoc.NewElement("var");
		solutionVarLevel->SetAttribute("name", "Flags");
		solutionVarLevel->SetText(solution.flags);
		solutionLevel->InsertEndChild(solutionVarLevel);

		solutionVarLevel = tempDoc.NewElement("var");
		solutionVarLevel->SetAttribute("name", "AlphaRef");
		solutionVarLevel->SetText(solution.alphaRef);
		solutionLevel->InsertEndChild(solutionVarLevel);

		solutionVarLevel = tempDoc.NewElement("var");
		solutionVarLevel->SetAttribute("name", "Hash");
		solutionVarLevel->SetText(solution.hash);
		solutionLevel->InsertEndChild(solutionVarLevel);


		Dx11VertexShaderPermutationSerializeXml(&tempDoc, solutionLevel, solution.vertexShader); 
		Dx11PixelShaderPermutationSerializeXml(&tempDoc, solutionLevel, solution.pixelShader);

		ShaderConstantSerializeXml(&tempDoc, solutionLevel, solution.vertexShaderResourceOverride, "VertexShaderConstantsBlockOverride");
		ShaderConstantSerializeXml(&tempDoc, solutionLevel, solution.pixelShaderResourceOverride, "PixelShaderConstantsBlockOverride");


		if (solution.solutionState->surfaceShaderId)
		{
			solutionStateVarLevel = tempDoc.NewElement("var");
			solutionStateVarLevel->SetAttribute("name", "surfaceShaderId");
			GuidToText((u8*)solution.solutionState->surfaceShaderId, onlyGuid);
			solutionStateVarLevel->SetText(onlyGuid);
			solutionStateLevel->InsertEndChild(solutionStateVarLevel);
		}
		if (solution.solutionState->vertexShaderFragmentId)
		{
			solutionStateVarLevel = tempDoc.NewElement("var");
			solutionStateVarLevel->SetAttribute("name", "vertexShaderFragmentId");
			GuidToText((u8*)solution.solutionState->vertexShaderFragmentId, onlyGuid);
			solutionStateVarLevel->SetText(onlyGuid);
			solutionStateLevel->InsertEndChild(solutionStateVarLevel);
		}
		if (solution.solutionState->geometryDeclarationId)
		{
			solutionStateVarLevel = tempDoc.NewElement("var");
			solutionStateVarLevel->SetAttribute("name", "geometryDeclarationId");
			GuidToText((u8*)solution.solutionState->geometryDeclarationId, onlyGuid);
			solutionStateVarLevel->SetText(onlyGuid);
			solutionStateLevel->InsertEndChild(solutionStateVarLevel);
		}

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "Mode");
		solutionStateVarLevel->SetText(ShaderRenderModeNames[solution.solutionState->mode]);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "Scaleable");
		solutionStateVarLevel->SetText(solution.solutionState->scaleable);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "GeometrySpace");
		solutionStateVarLevel->SetText(ShaderGeometrySpaceNames[solution.solutionState->geometrySpace]);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "SkinningMethod");
		solutionStateVarLevel->SetText(ShaderSkinningMethodNames[solution.solutionState->skinningMethod]);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "InstancingMethod");
		solutionStateVarLevel->SetText(ShaderInstancingMethodNames[solution.solutionState->instancingMethod]);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "Fog");
		solutionStateVarLevel->SetText(solution.solutionState->fog);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "ColorScale");
		solutionStateVarLevel->SetText(solution.solutionState->colorScale);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "Technique");
		solutionStateVarLevel->SetText(solution.solutionState->technique);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "PointLightCount");
		solutionStateVarLevel->SetText(solution.solutionState->pointLightCount);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "CustomBonesEnable");
		solutionStateVarLevel->SetText(solution.solutionState->customBonesEnable);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);

		solutionStateVarLevel = tempDoc.NewElement("var");
		solutionStateVarLevel->SetAttribute("name", "Ps3ClipPlaneCount");
		solutionStateVarLevel->SetText(solution.solutionState->ps3ClipPlaneCount);
		solutionStateLevel->InsertEndChild(solutionStateVarLevel);


		tinyxml2::XMLElement* pointLightSolutionBlock = tempDoc.NewElement("PointLightSolutionStatesBlock");

		for (int j = 0; j < 4; j++)
		{
			tinyxml2::XMLElement* pointLightSolutionLevel = tempDoc.NewElement("PointLightSolutionState");

			tinyxml2::XMLElement* pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "DiffuseEnable");
			pointLightSolutionVarLevel->SetText(solution.solutionState->pointLights[j].diffuseEnable);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "SpecularEnable");
			pointLightSolutionVarLevel->SetText(solution.solutionState->pointLights[j].specularEnable);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "ShadowBuffer");
			pointLightSolutionVarLevel->SetText(solution.solutionState->pointLights[j].shadowbuffer);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "ShadowMap");
			pointLightSolutionVarLevel->SetText(solution.solutionState->pointLights[j].shadowmap);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "Texture");
			pointLightSolutionVarLevel->SetText(solution.solutionState->pointLights[j].texture);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionVarLevel = tempDoc.NewElement("var");
			pointLightSolutionVarLevel->SetAttribute("name", "ShadowmapMethod");
			pointLightSolutionVarLevel->SetText(ShaderShadowmapMethodNames[solution.solutionState->pointLights[j].shadowmapMethod]);
			pointLightSolutionLevel->InsertEndChild(pointLightSolutionVarLevel);

			pointLightSolutionBlock->InsertEndChild(pointLightSolutionLevel);
		}
		solutionStateLevel->InsertEndChild(pointLightSolutionBlock);


		tinyxml2::XMLElement* outdoorLightSolutionStateLevel = tempDoc.NewElement("OutdoorLightSolutionState");
		tinyxml2::XMLElement* outdoorLightSolutionStateVarLevel = tempDoc.NewElement("var");
		outdoorLightSolutionStateVarLevel->SetAttribute("name", "Enable");
		outdoorLightSolutionStateVarLevel->SetText(solution.solutionState->outdoorLight.enable);
		outdoorLightSolutionStateLevel->InsertEndChild(outdoorLightSolutionStateVarLevel);

		outdoorLightSolutionStateVarLevel = tempDoc.NewElement("var");
		outdoorLightSolutionStateVarLevel->SetAttribute("name", "ShadowmapMethod");
		outdoorLightSolutionStateVarLevel->SetText(ShaderShadowmapMethodNames[solution.solutionState->outdoorLight.shadowmapMethod]);
		outdoorLightSolutionStateLevel->InsertEndChild(outdoorLightSolutionStateVarLevel);

		solutionStateLevel->InsertEndChild(outdoorLightSolutionStateLevel);
		solutionLevel->InsertEndChild(solutionStateLevel);
		varLevel->InsertEndChild(solutionLevel);
	}
	shaderLevel->InsertEndChild(varLevel);
	tempDoc.InsertFirstChild(decl);
	tempDoc.InsertAfterChild(decl, shaderLevel);
	tempDoc.SaveFile(fixedPath);
	return OK;
}



void Dx11PixelShaderPermutationSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, Dx11PixelShaderPermutation* permutation)
{
	char permutationId[GUIDSIZETEXT + 2];
	char tempGuid[GUIDSIZETEXT + 2];
	tinyxml2::XMLElement* level1 = doc->NewElement("Dx11PixelShaderPermutation");

	tinyxml2::XMLElement* level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "Id");
	GuidToText((u8*)permutation->id, permutationId);
	level1Var->SetText(permutationId);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "HLSLCodeSize");
	level1Var->SetText(permutation->shaderBytecodeSize);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "PixelShaderId");
	GuidToText(((u8*)permutation->shaderBytecode + 4), tempGuid);
	level1Var->SetText(tempGuid);
	level1->InsertEndChild(level1Var);

	ShaderFunctionConstantSerializeXml(doc, level1, permutation->constantFunctionBlock);
	ShaderTextureFunctionSerializeXml(doc, level1, permutation->textureFunctionBlock);
	ShaderConstantSerializeXml(doc, level1, permutation->constants, "ConstantsBlock");

	parent->InsertEndChild(level1);
	return;
}


void Dx11VertexShaderPermutationSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, Dx11VertexShaderPermutation* permutation)
{
	char permutationId[GUIDSIZETEXT + 2];
	char tempGuid[GUIDSIZETEXT + 2]; 
	tinyxml2::XMLElement* level1 = doc->NewElement("Dx11VertexShaderPermutation");

	tinyxml2::XMLElement* level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "Id");
	GuidToText((u8*)permutation->id, permutationId);
	level1Var->SetText(permutationId);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "HLSLCodeSize");
	level1Var->SetText(permutation->shaderBytecodeSize);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "VertexShaderId");
	GuidToText(((u8*)permutation->shaderBytecode + 4), tempGuid);
	level1Var->SetText(tempGuid);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "InputLayoutId");
	GuidToText(((u8*)permutation->shaderBytecodeInputSignature + 4), tempGuid);
	level1Var->SetText(tempGuid);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "InputSignatureSize");
	level1Var->SetText(permutation->shaderBytecodeInputSignatureSize);
	level1->InsertEndChild(level1Var);

	ShaderFunctionConstantSerializeXml(doc, level1, permutation->constantFunctionBlock); 
	ShaderTextureFunctionSerializeXml(doc, level1, permutation->textureFunctionBlock); 
	ShaderConstantSerializeXml(doc, level1, permutation->constants, "ConstantsBlock");

	parent->InsertEndChild(level1);
	return; 
}



void ShaderTextureFunctionSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderTextureFunctionBlock* functionBlock)
{
	tinyxml2::XMLElement* level1 = doc->NewElement("ShaderTextureFunctionBlock");

	tinyxml2::XMLElement* level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "TextureCount");
	level1Var->SetText(functionBlock->textureCount);
	level1->InsertEndChild(level1Var);

	tinyxml2::XMLElement* level2 = doc->NewElement("ShaderTextureFunctions");

	for (int i = 0; i < functionBlock->textureCount; i++)
	{
		tinyxml2::XMLElement* functionLevel = doc->NewElement("Function");

		tinyxml2::XMLElement* level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Function");
		level2Var->SetText(functionBlock->textures[i].function);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "ValueType");
		level2Var->SetText(functionBlock->textures[i].valueType);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Index");
		level2Var->SetText(functionBlock->textures[i].index);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Parameter");
		level2Var->SetText(functionBlock->textures[i].parameter);
		functionLevel->InsertEndChild(level2Var);

		level2->InsertEndChild(functionLevel);
	}
	level1->InsertEndChild(level2);
	parent->InsertEndChild(level1);
	return;
}


void ShaderFunctionConstantSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderConstantFunctionBlock* functionBlock)
{
	tinyxml2::XMLElement* level1 = doc->NewElement("ShaderConstantFunctionBlock");

	tinyxml2::XMLElement* level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "ConstantCount");
	level1Var->SetText(functionBlock->constantCount);
	level1->InsertEndChild(level1Var);

	level1Var = doc->NewElement("var");
	level1Var->SetAttribute("name", "RegisterCount");
	level1Var->SetText(functionBlock->registerCount);
	level1->InsertEndChild(level1Var);

	tinyxml2::XMLElement* level2 = doc->NewElement("ShaderConstantFunctions");
	for (int i = 0; i < functionBlock->constantCount; i++)
	{
		tinyxml2::XMLElement* functionLevel = doc->NewElement("Function");
		tinyxml2::XMLElement* level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Function");
		level2Var->SetText(ShaderConstantFunctionNames[functionBlock->constants[i].function]);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Index");
		level2Var->SetText(functionBlock->constants[i].index);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "Parameter");
		level2Var->SetText(functionBlock->constants[i].parameter);
		functionLevel->InsertEndChild(level2Var);

		level2Var = doc->NewElement("var");
		level2Var->SetAttribute("name", "ArraySize");
		level2Var->SetText(functionBlock->constants[i].arraySize);
		functionLevel->InsertEndChild(level2Var);

		level2->InsertEndChild(functionLevel);
    }
	level1->InsertEndChild(level2);
	parent->InsertEndChild(level1);
	return; 
}


void ShaderConstantSerializeXml(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, ShaderConstants* shaderConstants, const char* label)
{
	tinyxml2::XMLElement* shaderConstantLevel = doc->NewElement(label);

	tinyxml2::XMLElement* shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "ConstantCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_constantCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);

	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "ValueConstantStart");
	shaderConstantVarLevel->SetText(shaderConstants->m_valueConstantStart);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);

	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "ValueConstantCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_valueConstantCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);

	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "TextureConstantCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_textureConstantCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);


	if (shaderConstants->m_textureConstantCount > 0)
	{
		tinyxml2::XMLElement* textureBlock = doc->NewElement("TextureConstantBlock");
		for (int i = 0; i < shaderConstants->m_textureConstantCount; i++)
		{
			tinyxml2::XMLElement* textureLevel = doc->NewElement("TextureConstant");

			tinyxml2::XMLElement* textureVarLevel = doc->NewElement("var");
			textureVarLevel->SetAttribute("name", "Slot");
			textureVarLevel->SetText(shaderConstants->textureConstants[i].index);
			textureLevel->InsertEndChild(textureVarLevel);

			textureVarLevel = doc->NewElement("var");
			textureVarLevel->SetAttribute("name", "Type");
			textureVarLevel->SetText(shaderConstants->textureConstants[i].type);
			textureLevel->InsertEndChild(textureVarLevel);

			textureVarLevel = doc->NewElement("var");
			textureVarLevel->SetAttribute("name", "TexturePath");
			textureVarLevel->SetText(shaderConstants->textureConstants[i].textureName);
			textureLevel->InsertEndChild(textureVarLevel);

			textureBlock->InsertEndChild(textureLevel);
		}
		shaderConstantLevel->InsertEndChild(textureBlock);
	}

	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "ExternalValueConstantCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_externalValueConstantCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);


	if (shaderConstants->m_externalValueConstantCount > 0)
	{
		tinyxml2::XMLElement* Block = doc->NewElement("ExternalValueConstantBlock");
		for (int i = 0; i < shaderConstants->m_externalValueConstantCount; i++)
		{
			char defaultValue[256] = { 0 };
			tinyxml2::XMLElement* level = doc->NewElement("ExternalValueConstant");

			tinyxml2::XMLElement* varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Name");
			varLevel->SetText(shaderConstants->externalValueConstants[i].name);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Index");
			varLevel->SetText(shaderConstants->externalValueConstants[i].index);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Size");
			varLevel->SetText(shaderConstants->externalValueConstants[i].size);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Required");
			varLevel->SetText(shaderConstants->externalValueConstants[i].required);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Handle");
			varLevel->SetText(shaderConstants->externalValueConstants[i].handle);
			level->InsertEndChild(varLevel);

			snprintf(defaultValue, sizeof(defaultValue) - 1, "%f/%f/%f/%f", shaderConstants->externalValueConstants[i].defaultValue[0], shaderConstants->externalValueConstants[i].defaultValue[1],
				shaderConstants->externalValueConstants[i].defaultValue[2], shaderConstants->externalValueConstants[i].defaultValue[3]);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "DefaultValue");
			varLevel->SetText(defaultValue);
			level->InsertEndChild(varLevel);

			Block->InsertEndChild(level);

		}
		shaderConstantLevel->InsertEndChild(Block);
	}
	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "ExternalTextureConstantCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_externalTextureConstantCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);


	if (shaderConstants->m_externalTextureConstantCount > 0)
	{
		tinyxml2::XMLElement* Block = doc->NewElement("ExternalTextureConstantBlock");
		for (int i = 0; i < shaderConstants->m_externalTextureConstantCount; i++)
		{
			tinyxml2::XMLElement* level = doc->NewElement("ExternalTextureConstant");

			tinyxml2::XMLElement* varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Name");
			varLevel->SetText(shaderConstants->externalTextureConstants[i].name);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Index");
			varLevel->SetText(shaderConstants->externalTextureConstants[i].index);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Type");
			varLevel->SetText(shaderConstants->externalTextureConstants[i].type);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Handle");
			varLevel->SetText(shaderConstants->externalTextureConstants[i].handle);
			level->InsertEndChild(varLevel);


			Block->InsertEndChild(level);
		}
		shaderConstantLevel->InsertEndChild(Block);
	}
	shaderConstantVarLevel = doc->NewElement("var");
	shaderConstantVarLevel->SetAttribute("name", "SamplerStateCount");
	shaderConstantVarLevel->SetText(shaderConstants->m_samplerStateCount);
	shaderConstantLevel->InsertEndChild(shaderConstantVarLevel);

	if (shaderConstants->m_samplerStateCount > 0)
	{
		tinyxml2::XMLElement* Block = doc->NewElement("SamplerStatesBlock");
		for (int i = 0; i < shaderConstants->m_samplerStateCount; i++)
		{
			char BorderColorValue[256] = { 0 };
			tinyxml2::XMLElement* level = doc->NewElement("SamplerState");

			tinyxml2::XMLElement* varLevel = doc->NewElement("var");
			varLevel->SetAttribute("name", "Index");
			varLevel->SetText(shaderConstants->samplerStates[i].index);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "Filter");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.Filter);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "AddressU");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.AddressU);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "AddressV");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.AddressV);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "AddressW");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.AddressW);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "MipLODBias");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.MipLODBias);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "MaxAnisotropy");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.MaxAnisotropy);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "ComparisonFunc");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.ComparisonFunc);
			level->InsertEndChild(varLevel);

			snprintf(BorderColorValue, sizeof(BorderColorValue) - 1, "%f/%f/%f/%f", shaderConstants->samplerStates[i].sd.BorderColor[0], shaderConstants->samplerStates[i].sd.BorderColor[1],
				shaderConstants->samplerStates[i].sd.BorderColor[2], shaderConstants->samplerStates[i].sd.BorderColor[3]);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "BorderColor");
			varLevel->SetText(BorderColorValue);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "MinLOD");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.MinLOD);
			level->InsertEndChild(varLevel);

			varLevel = doc->NewElement("var");
			varLevel->SetAttribute("D3D11_SAMPLER_DESC", "MaxLOD");
			varLevel->SetText(shaderConstants->samplerStates[i].sd.MaxLOD);
			level->InsertEndChild(varLevel);

			Block->InsertEndChild(level);
		}
		shaderConstantLevel->InsertEndChild(Block);
	}
	parent->InsertEndChild(shaderConstantLevel);
	return;
}

void FragmentShadersSerializeXml(std::map<u32, FragmentShaderStruct*>* gFragmentShadersMap)
{
	char savePath[256] = { 0 };
	std::map<u32, FragmentShaderStruct*>::iterator it; 
	for (it = gFragmentShadersMap->begin(); it != gFragmentShadersMap->end(); it++)
	{
		FragmentShaderStruct* fragment = (FragmentShaderStruct*)it->second;
		snprintf(savePath, sizeof(savePath) - 1, "%s%s.xml", gOutputPath, fragment->name);
		bundle_directory_test(savePath);
		tinyxml2::XMLDocument doc; 
		tinyxml2::XMLElement* fragmentLevel = doc.NewElement("FragmentShader");
		tinyxml2::XMLElement* fragmentVarLevel = doc.NewElement("AvailableIds");
		fragmentLevel->InsertEndChild(fragmentVarLevel);

		for (int j = 0; j < fb_array_size(&fragment->availableIds); j++)
		{
			char tempGuid[GUIDSIZETEXT + 2] = { 0 };
			u8* pGuid = (u8*)fb_array_get(&fragment->availableIds, j);
			tinyxml2::XMLElement* idLevel = doc.NewElement("var");
			idLevel->SetAttribute("name", "Id");
			GuidToText(pGuid, tempGuid);
			idLevel->SetText(tempGuid);
			fragmentVarLevel->InsertEndChild(idLevel);
		}
		doc.InsertEndChild(fragmentLevel);
		doc.SaveFile(savePath);
	}
	return; 
}


void VertexShadersSerializeToFiles(rbtree* Dx11VertexPermutationsMap)
{
	char savePath[512] = { 0 };
	char rootPath[512] = { 0 };
	char guid[GUIDSIZETEXT + 2] = { 0 };
	std::vector<void*> permutations;
	RBTreeGetPointers(Dx11VertexPermutationsMap, RB_FIRST(Dx11VertexPermutationsMap), &permutations);
	snprintf(rootPath, sizeof(rootPath) - 1, "%sShaders/VertexShaders/", gOutputPath);
	bundle_directory_test(rootPath);
	for (int i = 0; i < permutations.size(); i++)
	{
		Dx11VertexShaderPermutation* permutation = (Dx11VertexShaderPermutation*)permutations.at(i); 
		GuidToText((u8*)permutation->id, guid);

		snprintf(savePath, sizeof(savePath) - 1, "%sShaders/VertexShaders/vs_%s.Dx11VertexShader", gOutputPath, guid);
		FILE* shaderCode = fopen(savePath, "w+b");
		if (shaderCode)
		{
			fwrite(permutation->shaderBytecode, 1, permutation->shaderBytecodeSize, shaderCode);
			fclose(shaderCode);
		}
		snprintf(savePath, sizeof(savePath) - 1, "%sShaders/VertexShaders/is_%s.Dx11InputSignature",gOutputPath, guid);
		shaderCode = fopen(savePath, "w+b");
		if (shaderCode)
		{
			fwrite(permutation->shaderBytecodeInputSignature, 1, permutation->shaderBytecodeInputSignatureSize, shaderCode);
			fwrite(&permutation->nInputLayoutElements, 1, 4, shaderCode);
			fwrite(permutation->inputLayoutElements, 1, permutation->nInputLayoutElements * sizeof(D3D11_INPUT_ELEMENT_DESC), shaderCode);
			fwrite(&permutation->nInputLayoutElements, 1, 4, shaderCode);
			for (int j = 0; j < permutation->nInputLayoutElements; j++)
			{
				i32 test = strlen(permutation->inputLayoutElements[j].SemanticName) + 1;
				fwrite(permutation->inputLayoutElements[j].SemanticName, 1, strlen(permutation->inputLayoutElements[j].SemanticName) + 1, shaderCode);
			}
			fclose(shaderCode);
		}
	}

	return; 
}


void PixelShadersSerializeToFiles(rbtree* Dx11PixelPermutationsMap)
{
	char savePath[512] = { 0 };
	char rootPath[512] = { 0 };
	char guid[GUIDSIZETEXT + 2] = { 0 };
	std::vector<void*> permutations;
	RBTreeGetPointers(Dx11PixelPermutationsMap, RB_FIRST(Dx11PixelPermutationsMap), &permutations);
	snprintf(rootPath, sizeof(rootPath) - 1, "%sShaders/PixelShaders/", gOutputPath);
	bundle_directory_test(rootPath);
	for (int i = 0; i < permutations.size(); i++)
	{
		Dx11PixelShaderPermutation* permutation = (Dx11PixelShaderPermutation*)permutations.at(i);
		GuidToText((u8*)permutation->id, guid);

		snprintf(savePath, sizeof(savePath) - 1, "%sShaders/PixelShaders/ps_%s.Dx11PixelShader", gOutputPath, guid);
		FILE* shaderCode = fopen(savePath, "w+b");
		if (shaderCode)
		{
			fwrite(permutation->shaderBytecode, 1, permutation->shaderBytecodeSize, shaderCode);
			fclose(shaderCode);
		}
	}

	return; 
}



void GeometryDeclarationSerializeXml(rbtree* geometryDeclMap)
{
	char savePath[512] = { 0 };
	char rootPath[512] = { 0 };
	char declarationGuid[GUIDSIZETEXT + 2] = { 0 };
	std::vector<void*> declarations;
	RBTreeGetPointers(geometryDeclMap, RB_FIRST(geometryDeclMap), &declarations);
	snprintf(rootPath, sizeof(rootPath) - 1, "%sShaders/GeometryDeclarations/", gOutputPath);
	bundle_directory_test(rootPath);

	for (int i = 0; i < declarations.size(); i++)
	{
		char tempValues[64] = { 0 };
		tinyxml2::XMLDocument doc;
		DICEGeometryDeclaration* decl = (DICEGeometryDeclaration*)declarations.at(i);
		GuidToText((u8*)decl->guid, declarationGuid);
		snprintf(savePath, sizeof(savePath) - 1, "%sShaders/GeometryDeclarations/%s.xml", gOutputPath, declarationGuid);
		tinyxml2::XMLElement* declLevel = doc.NewElement("GeometryDeclaration");

		tinyxml2::XMLElement* declVarLevel = doc.NewElement("var");
		declVarLevel->SetAttribute("name", "Id");
		declVarLevel->SetText(declarationGuid);
		declLevel->InsertEndChild(declVarLevel);
		tinyxml2::XMLElement* elementBlockLevel = doc.NewElement("Elements");
		for (int j = 0; j < decl->elementCount; j++)
		{
			tinyxml2::XMLElement* elementLevel = doc.NewElement("ElementDefinition");

			tinyxml2::XMLElement* elementVarLevel = doc.NewElement("var");
			elementVarLevel->SetAttribute("name", "Format");
			//		elementVarLevel->SetText(ShaderValueTypeNames[decl->elements[j].format]);
			elementVarLevel->SetText(decl->elements[j].format);
			elementLevel->InsertEndChild(elementVarLevel);

			elementVarLevel = doc.NewElement("var");
			elementVarLevel->SetAttribute("name", "Usage");
			elementVarLevel->SetText(ShaderVertexElementUsageNames[decl->elements[j].usage]);
			elementLevel->InsertEndChild(elementVarLevel);

			elementVarLevel = doc.NewElement("var");
			elementVarLevel->SetAttribute("name", "Offset");
			elementVarLevel->SetText(decl->elements[j].offset);
			elementLevel->InsertEndChild(elementVarLevel);

			elementVarLevel = doc.NewElement("var");
			elementVarLevel->SetAttribute("name", "StreamIndex");
			elementVarLevel->SetText(decl->elements[j].streamIndex);
			elementLevel->InsertEndChild(elementVarLevel);

			elementBlockLevel->InsertEndChild(elementLevel);
		}

		declLevel->InsertEndChild(elementBlockLevel);

		snprintf(tempValues, sizeof(tempValues), "%u/%u/%u/%u", decl->streamClassifications[0], decl->streamClassifications[1], decl->streamClassifications[2], decl->streamClassifications[3]);
		declVarLevel = doc.NewElement("var");
		declVarLevel->SetAttribute("name", "StreamClassifications");
		declVarLevel->SetText(tempValues);
		declLevel->InsertEndChild(declVarLevel);

		snprintf(tempValues, sizeof(tempValues), "%u/%u/%u/%u", decl->streamStrides[0], decl->streamStrides[1], decl->streamStrides[2], decl->streamStrides[3]);
		declVarLevel = doc.NewElement("var");
		declVarLevel->SetAttribute("name", "StreamStrides");
		declVarLevel->SetText(tempValues);
		declLevel->InsertEndChild(declVarLevel);

		doc.InsertEndChild(declLevel);
		doc.SaveFile(savePath);
	}
	return;
}


tinyxml2::XMLElement* XmlFindInstance(tinyxml2::XMLElement* instance_runner, char* target)
{
	char srcGuid[GUIDSIZETEXT + 2] = { 0 };
	char tempGuid[GUIDSIZETEXT + 2] = { 0 };
	memcpy(srcGuid, target, GUIDSIZETEXT);
	tinyxml2::XMLAttribute* atr;
	while (instance_runner)
	{
		atr = const_cast<tinyxml2::XMLAttribute*>(instance_runner->FirstAttribute());
		memcpy(tempGuid, atr->Value(), GUIDSIZETEXT);
		to_lower_inplace(tempGuid);
		if (memcmp(srcGuid, tempGuid, GUIDSIZETEXT) == 0)
		{
			return instance_runner;
		}
		instance_runner = instance_runner->NextSiblingElement();
	}
	return NULL;
}


const tinyxml2::XMLAttribute* XmlVarGetAttributeByName(tinyxml2::XMLElement* var, const char* targetName)
{
	char attribName[64] = { 0 };
	const tinyxml2::XMLAttribute* res = var->FirstAttribute();
	while (res != NULL)
	{
		strncpy(attribName, res->Name(), sizeof(attribName) - 1);
		to_lower_inplace(attribName);
		if (strcmp(attribName, targetName) == 0) return res;
		res = res->Next();
	}
}

tinyxml2::XMLElement* XmlInstanceGetVarByName(tinyxml2::XMLElement* instance, const char* targetName)
{
	char target[64] = { 0 };
	char varName[64] = { 0 };
	tinyxml2::XMLElement* res = NULL;
	res = instance->FirstChildElement();
	strncpy(target, targetName, sizeof(target) - 1);
	to_lower_inplace(target);
	for (int i = 0; i < instance->ChildElementCount(); i++)
	{
		const tinyxml2::XMLAttribute* attributeName = XmlVarGetAttributeByName(res, "name");
		if (attributeName != NULL)
		{
			strncpy(varName, attributeName->Value(), sizeof(varName) - 1);
			to_lower_inplace(varName);
			if (strcmp(varName, target) == 0) return res;
		}
		res = res->NextSiblingElement();
	}
	return res;
}

tinyxml2::XMLElement* XmlGetPrimaryInstance(tinyxml2::XMLDocument* doc)
{
	char lowercaseTargetGuid[GUIDSIZETEXT + 2] = { 0 };
	char lowercaseTempGuid[GUIDSIZETEXT + 2] = { 0 };
	tinyxml2::XMLElement* partition = doc->FirstChildElement();
	tinyxml2::XMLElement* instance = NULL;
	if (strcmp(partition->Value(), "partition") != 0) return NULL; 
	const char* primaryInstance = partition->FirstAttribute()->Next()->Value();
	memcpy(lowercaseTargetGuid, primaryInstance, GUIDSIZETEXT);
	to_lower_inplace(lowercaseTargetGuid);
	instance = partition->FirstChildElement();
	for (int i = 0; i < partition->ChildElementCount(); i++)
	{
		const char* instanceGuid = instance->FirstAttribute()->Value();
		memcpy(lowercaseTempGuid, instanceGuid, GUIDSIZETEXT);
		to_lower_inplace(lowercaseTempGuid);
		if (memcmp(lowercaseTargetGuid, lowercaseTempGuid, GUIDSIZETEXT) == 0)
		{
			return instance;
		}
		instance = instance->NextSiblingElement();
	}
	return NULL;
}

i32 XmlGetClassInstancesByName(tinyxml2::XMLDocument* doc, const char* targetName, tinyxml2::XMLElement** resClasses)
{
	i32 nClasses = 0;
	tinyxml2::XMLElement* partition = doc->FirstChildElement();
	tinyxml2::XMLElement* instance = NULL;
	instance = partition->FirstChildElement();
	for (int i = 0; i < partition->ChildElementCount(); i++)
	{
		char lowercaseClassName[96] = { 0 };
		const char* className = instance->FirstAttribute()->Next()->Value();
		strncpy(lowercaseClassName, className, sizeof(lowercaseClassName) - 1);
		to_lower_inplace(lowercaseClassName);
		if (strstr(lowercaseClassName, targetName))
		{
			resClasses[nClasses] = instance;
			nClasses++;
		}
		instance = instance->NextSiblingElement();
	}
	return nClasses;
}
