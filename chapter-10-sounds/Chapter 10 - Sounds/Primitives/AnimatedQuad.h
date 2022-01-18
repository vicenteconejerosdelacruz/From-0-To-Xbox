#pragma once
#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Constants/AnimatedQuadShaderConstants.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct AnimatedQuad {
  static constexpr UINT16 indices[] =
  {
     0, 1, 2, 3, 2, 1
  };

  typedef VertexPosTexCoord VertexType;
  static constexpr VertexType vertices[] = {
    { XMFLOAT3( 1.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3( 1.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },
  };
  
  bool loadingComplete = false;

  //buffers
  VertexBufferViewData<VertexType>                        vbvData;
  IndexBufferViewData                                     ibvData;
  ConstantsBufferViewData<AnimatedQuadShaderConstants>    cbvData;
  
  //textures
  ComPtr<ID3D12Resource>    texture;
  ComPtr<ID3D12Resource>    textureUpload;
  UINT                      numAnimatedFrames;
  FLOAT                     timePerFrames;
  FLOAT                     currentTime = 0.0f;
  UINT                      currentFrame = 0;
  FLOAT                     alphaCut;

  //shaders
  std::vector<byte>					vertexShader;
  std::vector<byte>					pixelShader;

  //pipeline state
  ComPtr<ID3D12RootSignature>   rootSignature;
  ComPtr<ID3D12PipelineState>   pipelineState;

  //transformations
  XMMATRIX world;

  void Initialize(UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, std::wstring path, UINT numFramesInTexture, FLOAT timeBetweenFrames, FLOAT alphaThreshold, DXGI_FORMAT textureFormat = DXGI_FORMAT_BC7_UNORM_SRGB);
  void DestroyUploadResources();
  void Destroy();
  void Step(FLOAT delta);
  void UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection);
  void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex);
};

