#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace DirectX;

ComPtr<IDXGIAdapter4> GetAdapter();
ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device);
ComPtr<IDXGISwapChain4> CreateSwapChain(CoreWindow^ window, ComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount);
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource> renderTargets[], UINT bufferCount);
void UpdateDepthStencilView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height);
ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device);
ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator);
ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);
HANDLE CreateEventHandle();
UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue);
void WaitForFenceValue(ComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent);
void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent);
void UpdateBufferResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData);