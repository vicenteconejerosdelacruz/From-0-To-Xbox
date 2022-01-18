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
void CreateTextureResource(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

//D3D11On12
void CreateD3D11On12Device(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D11Device>& d3d11Device, ComPtr<ID3D11On12Device>& d3d11on12Device, ComPtr<ID3D11DeviceContext>& d3d11DeviceContext, ComPtr<IDXGIDevice>& dxgiDevice);
void CreateD2D1Device(ComPtr<IDXGIDevice> dxgiDevice, ComPtr<ID2D1Factory6>& d2d1Factory, ComPtr<ID2D1Device5>& d2d1Device, ComPtr<ID2D1DeviceContext5>& d2d1DeviceContext);
template<size_t bufferCount>
void UpdateD2D1RenderTargets(ComPtr<ID3D11On12Device>	d3d11on12Device, ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID3D12Resource> renderTargets[], ComPtr<ID3D11Resource>(&wrappedBackBuffers)[bufferCount], ComPtr<ID2D1Bitmap1>(&d2dRenderTargets)[bufferCount]) {
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 144.0f, 144.0f
	);

	for (UINT i = 0; i < bufferCount; i++) {
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		DX::ThrowIfFailed(d3d11on12Device->CreateWrappedResource(
			renderTargets[i].Get(), &d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&wrappedBackBuffers[i])
		));

		ComPtr<IDXGISurface> surface;
		DX::ThrowIfFailed(wrappedBackBuffers[i].As(&surface));
		DX::ThrowIfFailed(d2d1DeviceContext->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, &d2dRenderTargets[i]));
	}
}