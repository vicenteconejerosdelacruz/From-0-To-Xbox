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

	//create a heap to store the location of 3(numFrames) constants buffers(mvp matrix) and 2 others for the texture
	//crear un heap para guardar las direcciones de 3(numFrames) buffers de constantes(matriz mvp) y 2 mas para la textura
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = numFrames+2;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&cbvsrvHeap)));
	NAME_D3D12_OBJECT(cbvsrvHeap);

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
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
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
	//once this loop is done, cbvCpuHandle will be used for the SRV of the texture
	//una vez que termino este loop, cbvCpuHandle se usara para el SRV de la textura

	//map the CPU memory to the GPU and then mapped memory
	//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
	CD3DX12_RANGE readRange(0, 0);
	DX::ThrowIfFailed(constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantBuffer)));
	ZeroMemory(mappedConstantBuffer, numFrames * alignedConstantBufferSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC crateSrvDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC fragileSrvDesc = {};
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE crateCpuHandle(cbvCpuHandle);
	CD3DX12_CPU_DESCRIPTOR_HANDLE fragileCpuHandle(cbvCpuHandle);
	fragileCpuHandle.Offset(cbvDescriptorSize);

	CreateTextureResource(d3dDevice, commandList, L"Assets/crate/crate.dds", crateTexture, crateTextureUpload, crateSrvDesc);
	CreateTextureResource(d3dDevice, commandList, L"Assets/crate/fragile.dds", fragileTexture, fragileTextureUpload, fragileSrvDesc);

	d3dDevice->CreateShaderResourceView(crateTexture.Get(), &crateSrvDesc, crateCpuHandle);
	d3dDevice->CreateShaderResourceView(fragileTexture.Get(), &fragileSrvDesc, fragileCpuHandle);

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\TexCoord.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\TexCoord.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {

		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_DESCRIPTOR_RANGE rangeSRV;
			CD3DX12_ROOT_PARAMETER parameter[2];

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			rangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
			parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_VERTEX);
			parameter[1].InitAsDescriptorTable(1, &rangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

			D3D12_STATIC_SAMPLER_DESC samplers[1];
			for (UINT i = 0; i < _countof(samplers); i++) {
				samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				samplers[i].MipLODBias = 0;
				samplers[i].MaxAnisotropy = 0;
				samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				samplers[i].MinLOD = 0.0f;
				samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
				samplers[i].ShaderRegister = i;
				samplers[i].RegisterSpace = 0;
				samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			}

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(_countof(parameter), parameter, _countof(samplers), samplers, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
			NAME_D3D12_OBJECT(rootSignature);
		}

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexType,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
	crateTextureUpload->Release();
	fragileTextureUpload->Release();
}

void Cube::Destroy() {
	crateTexture->Release();
	fragileTexture->Release();
	vertexBuffer->Release();
	indexBuffer->Release();
	constantBuffer->Release();
	cbvsrvHeap->Release();
	rootSignature->Release();
	pipelineState->Release();
}

void Cube::UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection)
{
	if (!loadingComplete) return;

	yRotation += yRotationStep;
	yTranslation += yTranslationStep;

	XMMATRIX world = XMMatrixMultiply(XMMatrixRotationY(yRotation), XMMatrixTranslation(0, 2*sinf(yTranslation), 0));
	XMMATRIX wvp = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));

	UINT8* destination = mappedConstantBuffer + (backBufferIndex * alignedConstantBufferSize);
	memcpy(destination, &wvp, sizeof(wvp));
}

void Cube::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	if (!loadingComplete) return;

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { cbvsrvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetPipelineState(pipelineState.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, cbvDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE texGpuHandle(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, cbvDescriptorSize);
	commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(1, texGpuHandle);
	
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}
