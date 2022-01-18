#pragma once

#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Constants/LightingShaderConstants.h"
#include "../Shaders/Constants/ShadowMapAlphaCutShaderConstants.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Static3DModel {

  bool loadingComplete = false;

  typedef VertexPosNormalTangentBiTangentTexCoord VertexType;
  std::vector<VertexBufferViewData<VertexType>>                                   vbvData;
  std::vector<IndexBufferViewData>                                                ibvData;
  std::vector<ConstantsBufferViewData<LightingShaderConstants>>                   cbvData;
  std::vector<ComPtr<ID3D12Resource>>                                             textures;
  std::vector<ComPtr<ID3D12Resource>>                                             texturesUpload;
  std::vector<BOOL>                                                               hasNormalMap;
  std::vector<UINT>                                                               meshIndexes;
  std::vector<UINT>                                                               twoSidedMeshIndexes;
  std::vector<UINT>                                                               alphaCutMeshIndexes;
  std::vector<UINT>                                                               alphaCutTwoSidedMeshIndexes;
  std::vector<FLOAT>                                                              alphaCut;
  std::vector<FLOAT>                                                              materialSpecularExponent;
  
  //constants buffers for holding shadow map worldViewProjection matrices
  //buffers de constantes para guardar las matrices de worldViewProjection de los shadowmaps
  std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>> directionalLightShadowMapCbvData;
  std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>> spotLightShadowMapCbvData;
  std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>> pointLightShadowMapCbvData[6];

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

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap, std::string path, std::set<UINT> skipMeshes = {});
  void DestroyUploadResources();
  void Destroy();
  void UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);

  void UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>>& shadowMapCbvData);
  void RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>>& shadowMapCbvData);
};

