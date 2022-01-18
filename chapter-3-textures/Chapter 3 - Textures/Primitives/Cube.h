#pragma once
#include "../Renderer/VertexFormats.h"

using namespace Microsoft::WRL;
using namespace DirectX;

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

  typedef VertexPosTexCoord VertexType;
  static constexpr VertexType vertices[] = {
    //+Y
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    //-X
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    //-Z
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

    //+X
    { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

    //+Z
    { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

    //-Y
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) }
  };

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
  static constexpr UINT     alignedConstantBufferSize = (sizeof(XMMATRIX) + 255) & ~255;

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
  static constexpr FLOAT        yRotationStep = 0.02f;
  FLOAT                         yRotation = 0.0f;
  //Y Axis Translation
  static constexpr FLOAT        yTranslationStep = 0.02f;
  FLOAT                         yTranslation = 0.0f;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList);
  void DestroyUploadResources();
  void Destroy();
  void UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);
};

