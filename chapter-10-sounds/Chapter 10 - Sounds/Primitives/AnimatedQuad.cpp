#include "pch.h"
#include "AnimatedQuad.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void AnimatedQuad::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, std::wstring path, UINT numFramesInTexture, FLOAT timeBetweenFrames, FLOAT alphaThreshold, DXGI_FORMAT textureFormat) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame, add one descriptor slot for the textures
	//crea los descriptors de constant buffer view por cada frame, agrega un slot de descriptore para la texturas
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 1U, d3dDevice, cbvData);
	D3D12_SHADER_RESOURCE_VIEW_DESC texSrvDesc = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE texCpuHandle(cbvCpuHandle);

	CreateTextureArrayResource(d3dDevice, commandList, (const LPWSTR)path.c_str(), texture, textureUpload, texSrvDesc, numFramesInTexture, textureFormat);

	d3dDevice->CreateShaderResourceView(texture.Get(), &texSrvDesc, texCpuHandle);

	numAnimatedFrames = numFramesInTexture;
	timePerFrames = timeBetweenFrames;
	alphaCut = alphaThreshold;

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\AnimatedQuad.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\AnimatedQuad.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {
		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_DESCRIPTOR_RANGE rangeSRV;
			CD3DX12_ROOT_PARAMETER parameter[2];

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			rangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);
			parameter[1].InitAsDescriptorTable(1, &rangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

			//define a sampler for the texture
			//define un sampler para el texture
			D3D12_STATIC_SAMPLER_DESC sampler;
			InitializeSampler(sampler, 0);

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(_countof(parameter), parameter, 1, &sampler, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
			NAME_D3D12_OBJECT(rootSignature);
		}

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexType,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		//initialize the pipeline state
		//inicializa el pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		InitializePipelineState(state, inputLayout, _countof(inputLayout), rootSignature, vertexShader, pixelShader);
		state.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));

		vertexShader.clear();
		pixelShader.clear();
		loadingComplete = true;
	});
}

void AnimatedQuad::DestroyUploadResources() {
	textureUpload && textureUpload->Release();
	vbvData.vertexBufferUpload->Release();
	ibvData.indexBufferUpload->Release();
}

void AnimatedQuad::Destroy() {
	texture->Release();
	vbvData.vertexBuffer->Release();
	ibvData.indexBuffer->Release();
	cbvData.constantBuffer->Release();
	cbvData.cbvsrvHeap->Release();
	rootSignature->Release();
	pipelineState->Release();
}

void AnimatedQuad::Step(FLOAT delta) {
	currentTime += delta;
	currentFrame = static_cast<UINT>(currentTime / timePerFrames) % numAnimatedFrames;
}

void AnimatedQuad::UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection)
{
	if (!loadingComplete) return;

	AnimatedQuadShaderConstants constants;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.index = currentFrame;
	constants.alphaCut = alphaCut;

	UINT8* destination = cbvData.mappedConstantBuffer + (backBufferIndex * cbvData.alignedConstantBufferSize);
	memcpy(destination, &constants, sizeof(constants));
}

void AnimatedQuad::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}
