#pragma once
#include "../Renderer/VertexFormats.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct ShaderConstants {
  UINT numTextures;
  BOOL useBlinnPhong;
  FLOAT materialSpecularExponent;
  XMMATRIX worldViewProjection;
  XMMATRIX world;
  XMVECTOR eyePos;
  XMVECTOR ambientLightColor;
  XMVECTOR directionalLightDirection;
  XMVECTOR directionalLightColor;
  XMVECTOR spotLightPosition;
  XMVECTOR spotLightColor;
  XMVECTOR spotLightDirectionAndAngle;
  XMVECTOR spotLightAttenuation;
  XMVECTOR pointLightColor;
  XMVECTOR pointLightPosition;
  XMVECTOR pointLightAttenuation;
};

struct Cube {
  static constexpr UINT16 indices[] =
  {
     0, 1, 2, 3, 2, 1, //+Y
     4, 5, 6, 7, 6, 5, //-X
     8, 9,10,11,10, 9, //-Z
    12,13,14,15,14,13, //+X
    16,17,18,19,18,17, //+Z
    20,21,22,23,22,21  //-Y
  };

  typedef VertexPosNormalTexCoord VertexType;
  static constexpr VertexType vertices[] = {
    //+Y
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    //-X
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    //-Z
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    //+X
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    //+Z
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f,  0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

    //-Y
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) }
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
  ComPtr<ID3D12Resource>    vertexBuffer;
  ComPtr<ID3D12Resource>    vertexBufferUpload;
  D3D12_VERTEX_BUFFER_VIEW  vertexBufferView;

  ComPtr<ID3D12Resource>    indexBuffer;
  ComPtr<ID3D12Resource>    indexBufferUpload;
  D3D12_INDEX_BUFFER_VIEW   indexBufferView;

  ComPtr<ID3D12Resource>    constantBuffer;
  UINT8*                    mappedConstantBuffer;
  static constexpr UINT     alignedConstantBufferSize = (sizeof(ShaderConstants) + 255) & ~255;

  //constant buffer view and shader resource view heap
  ComPtr<ID3D12DescriptorHeap>	cbvsrvHeap;
  UINT										      cbvDescriptorSize;

  //textures
  ComPtr<ID3D12Resource>    crateTexture;
  ComPtr<ID3D12Resource>    crateTextureUpload;
  ComPtr<ID3D12Resource>    fragileTexture;
  ComPtr<ID3D12Resource>    fragileTextureUpload;

  //shaders
  std::vector<byte>					vertexShader;
  std::vector<byte>					pixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>   pipelineState;

  //Y Axis Rotation
  static constexpr FLOAT        yRotationStep = 0.002f;
  FLOAT                         yRotation = 0.0f;
  //Y Axis Translation
  static constexpr FLOAT        yTranslationStep = 0.002f;
  static constexpr FLOAT        yOffset = 0.0f;
  FLOAT                         yTranslation = 0.0f;
  //Z Axis Translation
  static constexpr FLOAT        zTranslationStep = 0.0f;
  static constexpr FLOAT        zOffset = 0.0f;
  FLOAT                         zTranslation = 0.0f;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList);
  void DestroyUploadResources();
  void Destroy();
  void UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);
};

