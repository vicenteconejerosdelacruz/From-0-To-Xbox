#pragma once

#include "../Common/DirectXHelper.h"

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

//vertex buffer
template<typename T> struct VertexBufferViewData {
	ComPtr<ID3D12Resource>    vertexBuffer;
	ComPtr<ID3D12Resource>    vertexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW  vertexBufferView;
};

template<typename T>
void InitializeVertexBufferView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* vertices, UINT verticesCount, VertexBufferViewData<T> &vbvData) {
	UpdateBufferResource(d3dDevice, commandList, vbvData.vertexBuffer, vbvData.vertexBufferUpload, verticesCount, sizeof(T), vertices);
	vbvData.vertexBufferView.BufferLocation = vbvData.vertexBuffer->GetGPUVirtualAddress();
	vbvData.vertexBufferView.SizeInBytes = sizeof(T)* verticesCount;
	vbvData.vertexBufferView.StrideInBytes = sizeof(T);
	NAME_D3D12_OBJECT(vbvData.vertexBuffer);
}

//index buffer
struct IndexBufferViewData {
	ComPtr<ID3D12Resource>    indexBuffer;
	ComPtr<ID3D12Resource>    indexBufferUpload;
	D3D12_INDEX_BUFFER_VIEW   indexBufferView;
};

void InitializeIndexBufferView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData);

//constants buffers
template<typename T> struct ConstantsBufferViewData {
	static constexpr UINT alignedConstantBufferSize = (sizeof(T) + 255) & ~255;

	ComPtr<ID3D12Resource>        constantBuffer;
	UINT8* mappedConstantBuffer;
	ComPtr<ID3D12DescriptorHeap>	cbvsrvHeap;
	UINT										      cbvDescriptorSize;
};

template<typename T>
CD3DX12_CPU_DESCRIPTOR_HANDLE InitializeConstantsBufferView(UINT numFrames, UINT numTextures, ComPtr<ID3D12Device2> d3dDevice, ConstantsBufferViewData<T>& cbvData) {
	//create a heap to store the location of 3(numFrames) constants buffers(mvp matrix) and (numTextures) others for the texture
	//crear un heap para guardar las direcciones de 3(numFrames) buffers de constantes(matriz mvp) y (numTextures) mas para la textura
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = numFrames + numTextures;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&cbvData.cbvsrvHeap)));
	NAME_D3D12_OBJECT(cbvData.cbvsrvHeap);

	//create the constant buffer resource descriptor
	//crear el descriptor del constant buffer
	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numFrames * cbvData.alignedConstantBufferSize);
	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cbvData.constantBuffer)));
	NAME_D3D12_OBJECT(cbvData.constantBuffer);

	//get the address of the constant buffer in both GPU/CPU
	//trae las direcciones del buffer de constantes en la GPU/CPU
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = cbvData.constantBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(cbvData.cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	cbvData.cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create a constant buffer view for each frame in the backbuffer
	//crear un constant buffer view por cada frame en el backbuffer
	for (UINT n = 0; n < numFrames; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = cbvData.alignedConstantBufferSize;
		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

		cbvGpuAddress += desc.SizeInBytes;
		cbvCpuHandle.Offset(cbvData.cbvDescriptorSize);
	}
	//once this loop is done, cbvCpuHandle will be used for the SRV of the texture
	//una vez que termino este loop, cbvCpuHandle se usara para el SRV de la textura

	//map the CPU memory to the GPU and then mapped memory
	//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
	CD3DX12_RANGE readRange(0, 0);
	DX::ThrowIfFailed(cbvData.constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData.mappedConstantBuffer)));
	ZeroMemory(cbvData.mappedConstantBuffer, numFrames * cbvData.alignedConstantBufferSize);

	return cbvCpuHandle;
}

//samplers
void InitializeSampler(D3D12_STATIC_SAMPLER_DESC& sampler, UINT shaderRegister);

//pipe line state
void InitializePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::vector<byte>& vertexShader, std::vector<byte>& pixelShader);
void InitializeShadowMapPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::vector<byte>& vertexShader, std::vector<byte>& pixelShader);

//shadow map
void CreateShadowMapResourceView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12Resource> shadowMap, CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle);