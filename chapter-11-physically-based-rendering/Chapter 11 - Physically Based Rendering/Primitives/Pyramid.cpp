#include "pch.h"
#include "Pyramid.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void Pyramid::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame, add two descriptor slot for the textures and three more for ths shadowmaps
	//crea los descriptors de constant buffer view por cada frame, agrega dos slot de descriptores para las texturas y tres mas para los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 5U, d3dDevice, cbvData);

	D3D12_SHADER_RESOURCE_VIEW_DESC pyramidSrvDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC pyramidNormalMapSrvDesc = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE pyramidCpuHandle(cbvCpuHandle);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pyramidNormalMapCpuHandle(cbvCpuHandle);
	pyramidNormalMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CreateTextureResource(d3dDevice, commandList, L"Assets/pyramid/pyramid.dds", pyramidTexture, pyramidTextureUpload, pyramidSrvDesc);
	CreateTextureResource(d3dDevice, commandList, L"Assets/pyramid/pyramidNormalMap.dds", pyramidNormalMapTexture, pyramidNormalMapTextureUpload, pyramidNormalMapSrvDesc, DXGI_FORMAT_R8G8B8A8_UNORM);
	d3dDevice->CreateShaderResourceView(pyramidTexture.Get(), &pyramidSrvDesc, pyramidCpuHandle);
	d3dDevice->CreateShaderResourceView(pyramidNormalMapTexture.Get(), &pyramidNormalMapSrvDesc, pyramidNormalMapCpuHandle);

	//create the descriptors and shaders resource views for the shadowmaps
	//crea los descriptores y los shader resource views de los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCpuHandle(pyramidNormalMapCpuHandle);
	directionalLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCpuHandle(directionalLightShadowMapCpuHandle);
	spotLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCpuHandle(spotLightShadowMapCpuHandle);
	pointLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);

	CreateShadowMapResourceView(d3dDevice, directionalLightShadowMap, directionalLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, spotLightShadowMap, spotLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, pointLightShadowMap, pointLightShadowMapCpuHandle);

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\NormalMapping.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\NormalMapping.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {

		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_DESCRIPTOR_RANGE rangeSRV;
			CD3DX12_ROOT_PARAMETER parameter[2];

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			rangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
			parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);
			parameter[1].InitAsDescriptorTable(1, &rangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

			D3D12_STATIC_SAMPLER_DESC samplers[1];
			for (UINT i = 0; i < _countof(samplers); i++) {
				InitializeSampler(samplers[i], i);
			}

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(_countof(parameter), parameter, _countof(samplers), samplers, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())));
			NAME_D3D12_OBJECT(rootSignature);
		}

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexType,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//initialize the pipeline state
		//inicializa el pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		InitializePipelineState(state, inputLayout, _countof(inputLayout), rootSignature, vertexShader, pixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));

		vertexShader.clear();
		pixelShader.clear();
	});

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createShadowMapVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\ShadowMap.cso").then([this](std::vector<byte>& fileData) {
		shadowMapVertexShader = fileData;
	});
	auto createShadowMapPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\ShadowMap.cso").then([this](std::vector<byte>& fileData) {
		shadowMapPixelShader = fileData;
	});

	(createPipelineStateTask && createShadowMapVSTask && createShadowMapPSTask).then([this,d3dDevice,numFrames]() {

		InitializeConstantsBufferView(numFrames, 0U, d3dDevice, directionalLightShadowMapCbvData);
		InitializeConstantsBufferView(numFrames, 0U, d3dDevice, spotLightShadowMapCbvData);
		for (UINT i = 0; i < 6; i++) {
			InitializeConstantsBufferView(numFrames, 0U, d3dDevice, pointLightShadowMapCbvData[i]);
		}

		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_ROOT_PARAMETER parameter;

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			parameter.InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(shadowMapRootSignature.ReleaseAndGetAddressOf())));
			NAME_D3D12_OBJECT(shadowMapRootSignature);
		}

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//initialize the pipeline state for the shadow map
		//inicializa el pipeline state para el shadow map
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		InitializeShadowMapPipelineState(state, inputLayout, _countof(inputLayout), shadowMapRootSignature.Get(), shadowMapVertexShader, shadowMapPixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(shadowMapPipelineState.ReleaseAndGetAddressOf())));

		shadowMapVertexShader.clear();
		shadowMapPixelShader.clear();

		loadingComplete = true;
	});
}

void Pyramid::DestroyUploadResources() {
	vbvData.vertexBufferUpload->Release();
	ibvData.indexBufferUpload->Release();
	pyramidTextureUpload && pyramidTextureUpload->Release();
	pyramidNormalMapTextureUpload && pyramidNormalMapTextureUpload->Release();
}

void Pyramid::Destroy() {
	pyramidTexture->Release();
	pyramidNormalMapTexture->Release();
	vbvData.vertexBuffer->Release();
	ibvData.indexBuffer->Release();
	cbvData.constantBuffer->Release();
	cbvData.cbvsrvHeap->Release();
	directionalLightShadowMapCbvData.constantBuffer->Release();
	spotLightShadowMapCbvData.constantBuffer->Release();
	for (auto cbv : pointLightShadowMapCbvData) {
		cbv.constantBuffer->Release();
	}
	rootSignature->Release();
	pipelineState->Release();
}

void Pyramid::Step() {
	if (!loadingComplete) return;

	yRotation += yRotationStep;
	yTranslation += yTranslationStep;
	zTranslation += zTranslationStep;

	position = { XMVectorGetX(position), 2 * sinf(yTranslation) + yOffset, 2 * sinf(zTranslation) + zOffset , 0 };
}

void Pyramid::UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled)
{
	if (!loadingComplete) return;

	LightingShaderConstants constants;

	XMMATRIX world = XMMatrixMultiply(XMMatrixRotationY(yRotation), XMMatrixTranslationFromVector(position));
	constants.numTextures = 1;
	constants.useBlinnPhong = useBlinnPhong;
	constants.materialSpecularExponent = materialSpecularExponent;
	constants.normalMaps = normalMappingEnabled;
	constants.normalMapTextureIndex = 1;
	constants.shadowMapsTextureIndex = 2;
	constants.shadowMaps = shadowMapsEnabled;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.world = XMMatrixTranspose(world);
	constants.directionalLightShadowMapProjection = XMMatrixTranspose(directionalLightShadowMapProjection);
	constants.directionalLightShadowMapTexelInvSize = directionalLightShadowMapTexelInvSize;
	constants.spotLightShadowMapProjection = XMMatrixTranspose(spotLightShadowMapProjection);
	constants.spotLightShadowMapTexelInvSize = spotLightShadowMapTexelInvSize;
	for (UINT i = 0; i < 6U; i++) {
		constants.pointLightShadowMapProjection[i] = XMMatrixTranspose(pointLightShadowMapProjection[i]);
	}
	constants.pointLightShadowMapPartialDerivativeScale = 3.0f;
	constants.directionalLightShadowMapZBias = 0.0002f;
	constants.spotLightShadowMapZBias = 0.0001f;
	constants.pointLightShadowMapZBias = 0.0001f;
	constants.eyePos = eyePos;
	constants.ambientLightColor = ambientLightColor;
	constants.directionalLightDirection = directionalLightDirection;
	constants.directionalLightColor = directionalLightColor;
	constants.spotLightPosition = spotLightPosition;
	constants.spotLightColor = spotLightColor;
	constants.spotLightDirectionAndAngle = spotLightDirectionAndAngle;
	constants.spotLightAttenuation = spotLightAttenuation;
	constants.pointLightColor = pointLightColor;
	constants.pointLightPosition = pointLightPosition;
	constants.pointLightAttenuation = pointLightAttenuation;

	UINT8* destination = cbvData.mappedConstantBuffer + (backBufferIndex * cbvData.alignedConstantBufferSize);
	memcpy(destination, &constants, sizeof(constants));
}

void Pyramid::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}

void Pyramid::UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData) {
	if (!loadingComplete) return;

	XMMATRIX world = XMMatrixMultiply(XMMatrixRotationY(yRotation), XMMatrixTranslationFromVector(position));
	XMMATRIX shadowMapWorldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, shadowMapViewProjection));

	UINT8* destination = shadowMapCbvData.mappedConstantBuffer + (backBufferIndex * shadowMapCbvData.alignedConstantBufferSize);
	memcpy(destination, &shadowMapWorldViewProjection, sizeof(shadowMapWorldViewProjection));
}

void Pyramid::RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData) {
	Render3DPrimitiveShadowMap(commandList, backBufferIndex, shadowMapCbvData, *this);
}