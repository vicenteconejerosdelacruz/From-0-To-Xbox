#include "pch.h"
#include "Floor.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void Floor::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame plus three for shadowmaps
	//crea los descriptors de constant buffer view por cada frame mas tres para los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCpuHandle = InitializeConstantsBufferView(numFrames, 3U, d3dDevice, cbvData);
	CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCpuHandle(directionalLightShadowMapCpuHandle);
	spotLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCpuHandle(spotLightShadowMapCpuHandle);
	pointLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);

	CreateShadowMapResourceView(d3dDevice, directionalLightShadowMap, directionalLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, spotLightShadowMap, spotLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, pointLightShadowMap, pointLightShadowMapCpuHandle);

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\Floor.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\Floor.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {

		{
			CD3DX12_DESCRIPTOR_RANGE rangeCBV;
			CD3DX12_DESCRIPTOR_RANGE rangeSRV;
			CD3DX12_ROOT_PARAMETER parameter[2];

			rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			rangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);
			parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);
			parameter[1].InitAsDescriptorTable(1, &rangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

			//define a sampler for the shadow map
			//define un sampler para el shadow map
			D3D12_STATIC_SAMPLER_DESC shadowMapSampler;
			InitializeSampler(shadowMapSampler, 0);

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(_countof(parameter), parameter, 1, &shadowMapSampler, rootSignatureFlags);

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
		};

		//initialize the pipeline state
		//inicializa el pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		InitializePipelineState(state, inputLayout, _countof(inputLayout), rootSignature, vertexShader, pixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));

		vertexShader.clear();
		pixelShader.clear();
		loadingComplete = true;
	});
}

void Floor::DestroyUploadResources() {
	vbvData.vertexBufferUpload->Release();
	ibvData.indexBufferUpload->Release();
}

void Floor::Destroy() {
	vbvData.vertexBuffer->Release();
	ibvData.indexBuffer->Release();
	cbvData.constantBuffer->Release();
	cbvData.cbvsrvHeap->Release();
	rootSignature->Release();
	pipelineState->Release();
}

void Floor::UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6])
{
	if (!loadingComplete) return;

	LightingShaderConstants constants;

	XMMATRIX world = XMMatrixMultiply(XMMatrixScaling(10.0f, 1.0f, 10.0f), XMMatrixTranslation(0.0f, -2.0f, 0.0f));
	constants.useBlinnPhong = useBlinnPhong;
	constants.materialSpecularExponent = materialSpecularExponent;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.world = XMMatrixTranspose(world);
	constants.shadowMaps = shadowMapsEnabled;
	constants.directionalLightShadowMapProjection = XMMatrixTranspose(directionalLightShadowMapProjection);
	constants.directionalLightShadowMapTexelInvSize = directionalLightShadowMapTexelInvSize;
	constants.spotLightShadowMapProjection = XMMatrixTranspose(spotLightShadowMapProjection);
	constants.spotLightShadowMapTexelInvSize = spotLightShadowMapTexelInvSize;
	for (UINT i = 0; i < 6U; i++) {
		constants.pointLightShadowMapProjection[i] = XMMatrixTranspose(pointLightShadowMapProjection[i]);
	}
	constants.pointLightShadowMapPartialDerivativeScale = 3.0f;
	constants.directionalLightShadowMapZBias = 0.0f;
	constants.spotLightShadowMapZBias = 0.0f;
	constants.pointLightShadowMapZBias = 0.0f;
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

void Floor::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}
