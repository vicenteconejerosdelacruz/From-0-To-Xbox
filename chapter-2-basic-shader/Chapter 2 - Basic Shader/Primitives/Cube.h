#pragma once
#include "../Renderer/VertexFormats.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Cube {
  static constexpr UINT16 indices[] =
  {
      0, 1, 2, 0, 2, 3,
      4, 6, 5, 4, 7, 6,
      4, 5, 1, 4, 1, 0,
      3, 2, 6, 3, 6, 7,
      1, 5, 6, 1, 6, 2,
      4, 0, 3, 4, 3, 7
  };

  typedef VertexPosColor VertexType;
  static constexpr VertexType vertices[] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }
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

  //constant buffer view heap
  ComPtr<ID3D12DescriptorHeap>	cbvHeap;
  UINT										      cbvDescriptorSize;

  //shaders
  std::vector<byte>					vertexShader;
  std::vector<byte>					pixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>   pipelineState;

  //Y Axis Rotation
  static constexpr FLOAT        yRotationStep = 0.02f;
  FLOAT                         yRotation = 0.0f;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList);
  void DestroyUploadResources();
  void Destroy();
  void UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);
};

