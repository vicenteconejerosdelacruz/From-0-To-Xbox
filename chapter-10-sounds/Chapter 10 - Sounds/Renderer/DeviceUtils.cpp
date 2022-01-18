#include "pch.h"
#include "DeviceUtils.h"
#include "../Common/DirectXHelper.h"

ComPtr<IDXGIAdapter4> GetAdapter() {

	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;
	{
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
					D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				DX::ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
	ComPtr<ID3D12Device2> d3d12Device2;
	DX::ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&d3d12Device2)));

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
				D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		DX::ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return d3d12Device2;
}

ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	DX::ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

	return d3d12CommandQueue;
}

ComPtr<IDXGISwapChain4> CreateSwapChain(CoreWindow^ window, ComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount)
{
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = static_cast<int>(window->Bounds.Width);
	swapChainDesc.Height = static_cast<int>(window->Bounds.Height);
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	DX::ThrowIfFailed(
		dxgiFactory4->CreateSwapChainForCoreWindow(
			commandQueue.Get(), reinterpret_cast<IUnknown*>(window), &swapChainDesc, nullptr, &swapChain1
			)
		);

	DX::ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

	return dxgiSwapChain4;
}

ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
	return descriptorHeap;
}

void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource> renderTargets[], UINT bufferCount)
{
	auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < bufferCount; ++i) {
		ComPtr<ID3D12Resource> backBuffer;
		DX::ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		renderTargets[i] = backBuffer;
		rtvHandle.Offset(rtvDescriptorSize);
	}
}

void UpdateDepthStencilView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height)
{
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(&depthStencil)
		));
	NAME_D3D12_OBJECT(depthStencil);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
	dsv.Format = DXGI_FORMAT_D32_FLOAT;
	dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(depthStencil.Get(), &dsv, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device)
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator)
{
	ComPtr<ID3D12GraphicsCommandList2> commandList;
	DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
	DX::ThrowIfFailed(commandList->Close());
	return commandList;
}

ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device)
{
	ComPtr<ID3D12Fence> fence;
	DX::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	return fence;
}

HANDLE CreateEventHandle()
{
	HANDLE fenceEvent;
	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (fenceEvent == nullptr) {
		DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	return fenceEvent;
}

UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue)
{
	UINT64 fenceValueForSignal = ++fenceValue;
	DX::ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));
	return fenceValueForSignal;
}

void WaitForFenceValue(ComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent)
{
	//ask the fence what's it's current value and if's the GPU hasn't updated this value yet, make the CPU wait for it
	//le preguntamos al fence cual es su valor, y si la GPU aun no ha actualizado ese valor, que la CPU espere hasta que llegue
	if (fence->GetCompletedValue() < fenceValue)
	{
		DX::ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
	UINT64& fenceValue, HANDLE fenceEvent)
{
	UINT64 fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
	WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

void UpdateBufferResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12GraphicsCommandList2> commandList,
	ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData) {

	size_t bufferSize = numElements * elementSize;

	// Create a committed resource for the GPU resource in a default heap.
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&pDestinationResource)));

	NAME_D3D12_OBJECT(pDestinationResource);

	// Create a committed resource for the upload.
	if (bufferData)
	{
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pIntermediateResource)));

		NAME_D3D12_OBJECT(pIntermediateResource);

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(),
			pDestinationResource.Get(), pIntermediateResource.Get(),
			0, 0, 1, &subresourceData);
	}
}

static std::map<std::wstring, ComPtr<ID3D12Resource>> texturesCache;
static std::map<std::wstring, UINT> texturesCacheMipMaps;

void CreateTextureResource(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList,
	const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat) {

	UINT nMipMaps = 0U;
	if (texturesCache.find(path) != texturesCache.end()) {
		//if the texture is in the cache, take a new reference from it
		//si la textura esta en el cache, toma una nueva referencia de el
		texturesCache[path]->AddRef();
		texture = texturesCache[path];
		nMipMaps = texturesCacheMipMaps[path];
	} else {
		//to simplify things use dds image format and load trough DirectXTK12
		//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice.Get(), path, texture.ReleaseAndGetAddressOf(), ddsData, subresources));

		//obtain the size of the buffer which is SUM(1->n):mipmap(i)->width*height*4
		//obtener el porte del buffer el cual es SUM(1->n):mipmap(i)->width*height*4
		auto uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));

		//create the upload texture
		//creamos la textura de subida
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload)));

		//copy the data for the texture and it's mipmaps
		//copiamos los datos de la textura y sus mipmaps
		UpdateSubresources(commandList.Get(), texture.Get(), textureUpload.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

		nMipMaps = static_cast<UINT>(subresources.size());

		//store a reference to the texture in it's mipmaps in the cache
		//guardamos una referencia a la textura y sus mipmaps en el cache
		texturesCache[path] = texture;
		texturesCacheMipMaps[path] = nMipMaps;

		//now put a barrier for this copy to avoid keeping processing things untils this is done
		//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);
	}

	//create the descriptor for this texture
	//crear el descriptor de la textura
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureFormat;
	srvDesc.Texture2D.MipLevels = nMipMaps;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
}

void CreateTextureArrayResource(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList,
	const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, UINT numFrames, DXGI_FORMAT textureFormat) {

	UINT nMipMaps = 0U;
	if (texturesCache.find(path) != texturesCache.end()) {
		//if the texture is in the cache, take a new reference from it
		//si la textura esta en el cache, toma una nueva referencia de el
		texturesCache[path]->AddRef();
		texture = texturesCache[path];
		nMipMaps = texturesCacheMipMaps[path];
	} else {
		//to simplify things use dds image format and load through DirectXTK12
		//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice.Get(), path, texture.ReleaseAndGetAddressOf(), ddsData, subresources));

		//obtain the size of the buffer which is numFrames*SUM(1->n):mipmap(i)->width*height*4
		//obtener el porte del buffer el cual es numFrames*SUM(1->n):mipmap(i)->width*height*4
		auto uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));

		//create the upload texture
		//creamos la textura de subida
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload)));

		//copy the data for the texture and it's mipmaps
		//copiamos los datos de la textura y sus mipmaps
		UpdateSubresources(commandList.Get(), texture.Get(), textureUpload.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

		nMipMaps = static_cast<UINT>(subresources.size()) / numFrames;

		//store a reference to the texture in it's mipmaps in the cache
		//guardamos una referencia a la textura y sus mipmaps en el cache
		texturesCache[path] = texture;
		texturesCacheMipMaps[path] = nMipMaps;

		//now put a barrier for this copy to avoid keeping processing things untils this is done
		//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);
	}

	//create the descriptor for this texture
	//crear el descriptor de la textura
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureFormat;
	srvDesc.Texture2DArray.MipLevels = nMipMaps;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2DArray.ArraySize = numFrames;
}

void CreateD3D11On12Device(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D11Device>& d3d11Device, ComPtr<ID3D11On12Device>& d3d11on12Device, ComPtr<ID3D11DeviceContext>& d3d11DeviceContext, ComPtr<IDXGIDevice>& dxgiDevice) {
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DX::ThrowIfFailed(D3D11On12CreateDevice(d3dDevice.Get(), d3d11DeviceFlags,
		nullptr, 0, reinterpret_cast<IUnknown**>(commandQueue.GetAddressOf()),
		1, 0, &d3d11Device, &d3d11DeviceContext, nullptr));

	DX::ThrowIfFailed(d3d11Device.As(&d3d11on12Device));
	DX::ThrowIfFailed(d3d11on12Device.As(&dxgiDevice));
}

void CreateD2D1Device(ComPtr<IDXGIDevice> dxgiDevice, ComPtr<ID2D1Factory6>& d2d1Factory, ComPtr<ID2D1Device5>& d2d1Device, ComPtr<ID2D1DeviceContext5>& d2d1DeviceContext) {
	D2D1_FACTORY_OPTIONS d2d1FactoryOptions = {};
#if defined(_DEBUG)
	d2d1FactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	DX::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory6), &d2d1FactoryOptions, &d2d1Factory));
	DX::ThrowIfFailed(d2d1Factory->CreateDevice(dxgiDevice.Get(), &d2d1Device));
	DX::ThrowIfFailed(d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d1DeviceContext));
}

void InitializeIndexBufferView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData) {
	UpdateBufferResource(d3dDevice, commandList, ibvData.indexBuffer, ibvData.indexBufferUpload, indicesCount, sizeof(UINT16), indices);
	ibvData.indexBufferView.BufferLocation = ibvData.indexBuffer->GetGPUVirtualAddress();
	ibvData.indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	ibvData.indexBufferView.SizeInBytes = sizeof(UINT16)*indicesCount;
	NAME_D3D12_OBJECT(ibvData.indexBuffer);
}

void InitializeSampler(D3D12_STATIC_SAMPLER_DESC &sampler, UINT shaderRegister) {
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = shaderRegister;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void InitializePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::vector<byte>& vertexShader, std::vector<byte>& pixelShader) {
	state.InputLayout = { inputLayout, inputLayoutSize };
	state.pRootSignature = rootSignature.Get();
	state.VS = CD3DX12_SHADER_BYTECODE(&vertexShader[0], vertexShader.size());
	state.PS = CD3DX12_SHADER_BYTECODE(&pixelShader[0], pixelShader.size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	state.SampleDesc.Count = 1;
}

void InitializeShadowMapPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::vector<byte>& vertexShader, std::vector<byte>& pixelShader) {
	InitializePipelineState(state, inputLayout, inputLayoutSize, rootSignature, vertexShader, pixelShader);
	state.NumRenderTargets = 0;
	state.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
}

void CreateShadowMapResourceView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12Resource> shadowMap, CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle) {
	//create the shader resource view descriptor using R32_FLOAT format
	//crea el descriptor del shader resource view usando el formato R32_FLOAT
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSrvDesc.Texture2D.MipLevels = 1U;
	shadowSrvDesc.Texture2D.MostDetailedMip = 0;
	shadowSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	d3dDevice->CreateShaderResourceView(shadowMap.Get(), &shadowSrvDesc, shadowMapCpuHandle);
}
