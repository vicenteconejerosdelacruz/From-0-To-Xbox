#pragma once
using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace DirectX;

struct Renderer
{
	static const UINT frameCount = 3;
	static const constexpr float fovAngleY = (70.0f * XM_PI / 180.0f);

	//app window reference
	Agile<CoreWindow>                       window;

	//d3d device resources in order of creation
	ComPtr<ID3D12Device2>                   d3dDevice;
	ComPtr<ID3D12CommandQueue>              commandQueue;
	ComPtr<IDXGISwapChain4>                 swapChain;
	ComPtr<ID3D12DescriptorHeap>            rtvDescriptorHeap;
	UINT                                    rtvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap>            dsvDescriptorHeap;
	ComPtr<ID3D12CommandAllocator>          commandAllocators[frameCount];
	ComPtr<ID3D12GraphicsCommandList2>      commandList;

	//GPU <-> CPU synchronization 
	ComPtr<ID3D12Fence>                     fence;
	UINT64                                  fenceValue = 0;
	UINT64                                  frameFenceValues[frameCount] = {};
	HANDLE                                  fenceEvent;

	//window based values
	D3D12_VIEWPORT                          screenViewport;
	D3D12_RECT                              scissorRect;

	//back buffer and depth stencil targets
	ComPtr<ID3D12Resource>                  renderTargets[frameCount];
	ComPtr<ID3D12Resource>                  depthStencil;
	UINT                                    backBufferIndex;

	//matrices
	XMMATRIX                                perspectiveMatrix;
	
	void Initialize(CoreWindow^ coreWindow);
	void Destroy();
	void UpdateViewportPerspective();
	void Resize(UINT width, UINT height);
	void ResetCommands();
	void CloseCommandsAndFlush();
	void SetRenderTargets();
	void Present();
};

