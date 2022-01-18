#pragma once

#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Constants/SkinningShaderConstants.h"
#include "../Shaders/Constants/SkinningShadowMapAlphaCutShaderConstants.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include "../Utils/AssimpSkinning.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Animated3DModel {

  bool loadingComplete = false;

  typedef VertexPosNormalTangentBiTangentTexCoordSkinning VertexType;
  std::vector<VertexBufferViewData<VertexType>>                                   vbvData;
  std::vector<IndexBufferViewData>                                                ibvData;
  std::vector<ConstantsBufferViewData<SkinningShaderConstants>>                   cbvData;
  std::vector<ComPtr<ID3D12Resource>>                                             textures;
  std::vector<ComPtr<ID3D12Resource>>                                             texturesUpload;
  std::vector<BOOL>                                                               hasNormalMap;
  std::vector<UINT>                                                               meshIndexes;
  std::vector<UINT>                                                               twoSidedMeshIndexes;
  std::vector<UINT>                                                               alphaCutMeshIndexes;
  std::vector<UINT>                                                               alphaCutTwoSidedMeshIndexes;
  std::vector<FLOAT>                                                              alphaCut;
  
  //constants buffers for holding shadow map worldViewProjection matrices
  //buffers de constantes para guardar las matrices de worldViewProjection de los shadowmaps
  std::vector<ConstantsBufferViewData<SkinningShadowMapAlphaCutShaderConstants>> directionalLightShadowMapCbvData;
  std::vector<ConstantsBufferViewData<SkinningShadowMapAlphaCutShaderConstants>> spotLightShadowMapCbvData;
  std::vector<ConstantsBufferViewData<SkinningShadowMapAlphaCutShaderConstants>> pointLightShadowMapCbvData[6];

  //animation
  std::map<std::string, FLOAT>                            animationsLength;
  std::map<std::string, BoneInfo>                         boneInfo;
  XMMATRIX                                                rootNodeInverseTransform;
  std::string                                             currentAnimation = "";
  FLOAT                                                   currentAnimationTime = 0.0f;

  std::map<std::string, std::map<std::string, BoneKeys>>  animationsChannelsKeys;
  HierarchyNode                                           rootHierarchy;

  //shaders
  std::vector<byte>         vertexShader;
  std::vector<byte>         pixelShader;
  std::vector<byte>         shadowMapVertexShader;
  std::vector<byte>         shadowMapPixelShader;
  std::vector<byte>         shadowMapAlphaCutVertexShader;
  std::vector<byte>         shadowMapAlphaCutPixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>   pipelineState;
  ComPtr<ID3D12PipelineState>   twoSidedPipelineState;
  //pipeline state for shadow mapping
  ComPtr<ID3D12RootSignature>   shadowMapRootSignature;
  ComPtr<ID3D12PipelineState>   shadowMapPipelineState;
  ComPtr<ID3D12PipelineState>   shadowMapTwoSidedPipelineState;
  ComPtr<ID3D12RootSignature>   shadowMapAlphaCutRootSignature;
  ComPtr<ID3D12PipelineState>   shadowMapAlphaCutPipelineState;
  ComPtr<ID3D12PipelineState>   shadowMapAlphaCutTwoSidedPipelineState;
 
  //transformations
  XMMATRIX world;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap, std::string path);
  void DestroyUploadResources();
  void Destroy();
  void Step(FLOAT delta);
  void UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);

  void UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, std::vector<ConstantsBufferViewData<SkinningShadowMapAlphaCutShaderConstants>>& shadowMapCbvData);
  void RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, std::vector<ConstantsBufferViewData<SkinningShadowMapAlphaCutShaderConstants>>& shadowMapCbvData);
};

