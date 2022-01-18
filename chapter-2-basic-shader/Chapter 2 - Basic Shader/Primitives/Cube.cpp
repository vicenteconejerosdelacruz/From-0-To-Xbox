#include "pch.h"
#include "Cube.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void Cube::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	UpdateBufferResource(d3dDevice, commandList, vertexBuffer, vertexBufferUpload, _countof(vertices), sizeof(VertexType), vertices);
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices);
	vertexBufferView.StrideInBytes = sizeof(VertexType);
	NAME_D3D12_OBJECT(vertexBuffer);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	UpdateBufferResource(d3dDevice, commandList, indexBuffer, indexBufferUpload ,_countof(indices), sizeof(UINT16), indices);
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	NAME_D3D12_OBJECT(indexBuffer);

	//create a heap to store the location of 3(numFrames) constants buffers(mvp matrix)
	//crear un heap para guardar las direcciones de 3(numFrames) buffers de constantes(matriz mvp)
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = numFrames;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
	NAME_D3D12_OBJECT(cbvHeap);

	//create the constant buffer resource descriptor
	//crear el descriptor del constant buffer
	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numFrames * alignedConstantBufferSize);
	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)));
	NAME_D3D12_OBJECT(constantBuffer);

	//get the address of the constant buffer in both GPU/CPU
	//trae las direcciones del buffer de constantes en la GPU/CPU
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = constantBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(cbvHeap->GetCPUDescriptorHandleForHeapStart());
	cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create a constant buffer view for each frame in the backbuffer
	//crear un constant buffer view por cada frame en el backbuffer
	for (UINT n = 0; n < numFrames; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = alignedConstantBufferSize;
		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

		cbvGpuAddress += desc.SizeInBytes;
		cbvCpuHandle.Offset(cbvDescriptorSize);
	}

	//map the CPU memory to the GPU and then mapped memory
	//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
	CD3DX12_RANGE readRange(0, 0);
	DX::ThrowIfFailed(constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantBuffer)));
	ZeroMemory(mappedConstantBuffer, numFrames * alignedConstantBufferSize);

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\Color.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\Color.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {
		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_ROOT_PARAMETER parameter[1];

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_VERTEX);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(_countof(parameter), parameter, 0, nullptr, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
			NAME_D3D12_OBJECT(rootSignature);
		}

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { inputLayout, _countof(inputLayout) };
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

		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));

		vertexShader.clear();
		pixelShader.clear();
		loadingComplete = true;
	});
}

void Cube::DestroyUploadResources() {
	vertexBufferUpload->Release();
	indexBufferUpload->Release();
}

void Cube::Destroy() {
	vertexBuffer->Release();
	indexBuffer->Release();
	constantBuffer->Release();
	cbvHeap->Release();
	rootSignature->Release();
	pipelineState->Release();
}

void Cube::UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection)
{
	if (!loadingComplete) return;

	yRotation += yRotationStep;
	XMMATRIX world = XMMatrixRotationY(yRotation);
	XMMATRIX wvp = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));

	UINT8* destination = mappedConstantBuffer + (backBufferIndex * alignedConstantBufferSize);
	memcpy(destination, &wvp, sizeof(wvp));
}

void Cube::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	if (!loadingComplete) return;

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetPipelineState(pipelineState.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(cbvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, cbvDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);
	
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}
