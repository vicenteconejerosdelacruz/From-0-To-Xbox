#pragma once
#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Constants/LightingShaderConstants.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Pyramid {
  static constexpr UINT16 indices[] =
  {
     0, 1, 2,
     3, 4, 5,
     6, 7, 8,
     9,10,11,
    12,13,14,
    15,16,17
  };

  typedef VertexPosNormalTangentTexCoord VertexType;
  static constexpr VertexType vertices[] = {
    { XMFLOAT3( 1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f,  0.4472136f,  0.8944272f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 0.0f, 2.0f,  0.0f), XMFLOAT3(0.0f,  0.4472136f,  0.8944272f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f,  0.4472136f,  0.8944272f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 0.0f, 2.0f,  0.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3( 1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },
    
    { XMFLOAT3( 1.0f, 0.0f, -1.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 0.0f, 2.0f,  0.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0,  1.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3( 1.0f, 0.0f,  1.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0,  1.0f), XMFLOAT2(0.0f, 1.0f) },
                            
    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 0.0f, 2.0f,  0.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0, -1.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3( 0.0f,  0.0, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3( 1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3( 1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 0.0f) },
  };
  
  //keep a table of different exponents just to play around
  //mantenemos una tabla con diferentes exponentes solo para jugar
  //static constexpr FLOAT materialSpecularExponent = 1.0f;
  //static constexpr FLOAT materialSpecularExponent = 16.0f;
  //static constexpr FLOAT materialSpecularExponent = 64.0f;
  //static constexpr FLOAT materialSpecularExponent = 256.0f;
  static constexpr FLOAT materialSpecularExponent = 1024.0f;

  bool loadingComplete = false;

  //buffers
  VertexBufferViewData<VertexType>                          vbvData;
  IndexBufferViewData                                       ibvData;
  ConstantsBufferViewData<LightingShaderConstants>          cbvData;
  ConstantsBufferViewData<XMMATRIX>                         directionalLightShadowMapCbvData;
  ConstantsBufferViewData<XMMATRIX>                         spotLightShadowMapCbvData;
  ConstantsBufferViewData<XMMATRIX>                         pointLightShadowMapCbvData[6];

  //textures
  ComPtr<ID3D12Resource>    pyramidTexture;
  ComPtr<ID3D12Resource>    pyramidTextureUpload;
  ComPtr<ID3D12Resource>    pyramidNormalMapTexture;
  ComPtr<ID3D12Resource>    pyramidNormalMapTextureUpload;

  //shaders
  std::vector<byte>         vertexShader;
  std::vector<byte>         pixelShader;
  std::vector<byte>         shadowMapVertexShader;
  std::vector<byte>         shadowMapPixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>   pipelineState;
  ComPtr<ID3D12RootSignature>   shadowMapRootSignature;
  ComPtr<ID3D12PipelineState>   shadowMapPipelineState;

  XMVECTOR                      position;

  //Y Axis Rotation
  static constexpr FLOAT        yRotationStep = 0.0f;
  FLOAT                         yRotation = 0;
  //Y Axis Translation
  static constexpr FLOAT        yTranslationStep = 0.0f;
  static constexpr FLOAT        yOffset = -1.0f;
  FLOAT                         yTranslation = 0;
  //Z Axis Translation
  static constexpr FLOAT        zTranslationStep = 0.0f;
  static constexpr FLOAT        zOffset = -2.0f;
  FLOAT                         zTranslation = 0;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap);
  void DestroyUploadResources();
  void Destroy();
  void Step();
  void UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);

  void UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData);
  void RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData);
};

