#pragma once
#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Constants/LightingShaderConstants.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Floor {
  static constexpr UINT16 indices[] =
  {
     0, 1, 2, 3, 2, 1 //+Y
  };

  typedef VertexPosNormal VertexType;
  static constexpr VertexType vertices[] = {
    //+Y
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) }
  };

  //keep a table of different exponents just to play around
  //mantenemos una tabla con diferentes exponentes solo para jugar
  //static constexpr FLOAT materialSpecularExponent = 1.0f;
  //static constexpr FLOAT materialSpecularExponent = 16.0f;
  static constexpr FLOAT materialSpecularExponent = 64.0f;
  //static constexpr FLOAT materialSpecularExponent = 256.0f;
  //static constexpr FLOAT materialSpecularExponent = 1024.0f;

  bool loadingComplete = false;

  //buffers
  VertexBufferViewData<VertexType>                  vbvData;
  IndexBufferViewData                               ibvData;
  ConstantsBufferViewData<LightingShaderConstants>  cbvData;

  //shaders
  std::vector<byte>					vertexShader;
  std::vector<byte>					pixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>		pipelineState;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap);
  void DestroyUploadResources();
  void Destroy();
  void UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6]);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);
};

