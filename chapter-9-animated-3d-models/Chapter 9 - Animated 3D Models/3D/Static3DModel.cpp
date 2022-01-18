#include "pch.h"
#include "Static3DModel.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"
#include "../Renderer/Render3D.h"
#include "../Utils/AssetsUtils.h"

void Static3DModel::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap, std::string path) {

	Assimp::Importer importer;
	const aiScene* aiModel = importer.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!aiModel) {
		OutputDebugStringA(importer.GetErrorString());
		return;
	}

	//go through all the meshes in the model
	//recorre todos las mallas del modelo
	for (UINT meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++) {
		auto mesh = aiModel->mMeshes[meshIndex];

		VertexType* vertexData = new VertexType[mesh->mNumVertices]();
		//copy every vertex in the mesh
		//copia todos los vertices de la malla
		for (UINT vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
			//copy position data
			//copia la posicion
			vertexData[vertexIndex].Position.x = mesh->mVertices[vertexIndex][0];
			vertexData[vertexIndex].Position.y = mesh->mVertices[vertexIndex][1];
			vertexData[vertexIndex].Position.z = mesh->mVertices[vertexIndex][2];

			//copy normal data
			//copia la normal
			vertexData[vertexIndex].Normal.x = mesh->mNormals[vertexIndex][0];
			vertexData[vertexIndex].Normal.y = mesh->mNormals[vertexIndex][1];
			vertexData[vertexIndex].Normal.z = mesh->mNormals[vertexIndex][2];

			//copy tangent data
			//copia la tangente
			vertexData[vertexIndex].Tangent.x = mesh->mTangents[vertexIndex][0];
			vertexData[vertexIndex].Tangent.y = mesh->mTangents[vertexIndex][1];
			vertexData[vertexIndex].Tangent.z = mesh->mTangents[vertexIndex][2];

			//copy bitangent data
			//copia la bitangente
			vertexData[vertexIndex].BiTangent.x = mesh->mBitangents[vertexIndex][0];
			vertexData[vertexIndex].BiTangent.y = mesh->mBitangents[vertexIndex][1];
			vertexData[vertexIndex].BiTangent.z = mesh->mBitangents[vertexIndex][2];

			//copy texture coordinates data
			//copia las coordenadas de textura
			vertexData[vertexIndex].TexCoord.x = mesh->mTextureCoords[0][vertexIndex][0];
			vertexData[vertexIndex].TexCoord.y = mesh->mTextureCoords[0][vertexIndex][1];
		}

		VertexBufferViewData<VertexType> vbvMeshData;
		//upload the vertex buffer to the GPU and create the vertex buffer view
		//subir el vertex buffer a la GPU y crear el vertex buffer view
		InitializeVertexBufferView(d3dDevice, commandList, vertexData, mesh->mNumVertices, vbvMeshData);
		vbvData.push_back(vbvMeshData);

		//copy the faces data
		//copiar las caras
		UINT totalFaces = mesh->mNumFaces * mesh->mFaces[0].mNumIndices;
		UINT16* facesData = new UINT16[totalFaces]();
		UINT faceIndex = 0;
		for (UINT meshFaceIndex = 0; meshFaceIndex < mesh->mNumFaces; meshFaceIndex++) {
			for (UINT index = 0; index < mesh->mFaces[meshFaceIndex].mNumIndices; index++) {
				facesData[faceIndex] = static_cast<UINT16>(mesh->mFaces[meshFaceIndex].mIndices[index]);
				faceIndex++;
			}
		}

		//if this jump totalFaces is wrongly calculated
		//si esto salta totalFaces esta mal calculado
		assert(totalFaces == faceIndex);

		IndexBufferViewData ibvMeshData;
		//upload the index buffer to the GPU and create the index buffer view
		//subir el index buffer a la GPU y crear el index buffer view
		InitializeIndexBufferView(d3dDevice, commandList, facesData, faceIndex, ibvMeshData);
		ibvData.push_back(ibvMeshData);

		//take the diffuse texture and the normal map from the material
		//extrae la textura difusa y el normal map de los materiales
		auto material = aiModel->mMaterials[mesh->mMaterialIndex];
		aiString diffuseName, normalMapName;
		std::wstring diffusePath, normalMapPath;
		ComPtr<ID3D12Resource> diffuseTexture, diffuseTextureUpload, normalMapTexture, normalMapTextureUpload;
		D3D12_SHADER_RESOURCE_VIEW_DESC diffuseSrvDesc = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC normalMapSrvDesc = {};
		UINT numTextures = 0;

		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseName);
		if (diffuseName.length > 0) {
			//get the path of diffuse texture, but convert it to a model relative dds file
			//obtiene la ruta de la textura difusa, pero la convertimos a una ruta relativa al modelo en archivo dds
			diffusePath = assetTexturePath(path, diffuseName.C_Str());
			CreateTextureResource(d3dDevice, commandList, (const LPWSTR)diffusePath.c_str(), diffuseTexture, diffuseTextureUpload, diffuseSrvDesc);
			textures.push_back(diffuseTexture);
			texturesUpload.push_back(diffuseTextureUpload);
			numTextures++;
		}

		material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
		if (normalMapName.length > 0) {
			//get the path of normal map, but convert it to a model relative dds file
			//obtiene la ruta del normal map, pero la convertimos a una ruta relativa al modelo en archivo dds
			normalMapPath = assetTexturePath(path, normalMapName.C_Str());
			CreateTextureResource(d3dDevice, commandList, (const LPWSTR)normalMapPath.c_str(), normalMapTexture, normalMapTextureUpload, normalMapSrvDesc, DXGI_FORMAT_R8G8B8A8_UNORM);
			textures.push_back(normalMapTexture);
			texturesUpload.push_back(normalMapTextureUpload);
			numTextures++;
		}

		//get the alpha mode and alpha cut information
		//saca la informacion del modo de alpha y el alpha cut
		aiString alphaMode;
		ai_real alphaCutOff;
		material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
		BOOL isOpaque = strcmp(alphaMode.C_Str(), "OPAQUE") == 0;
		if (isOpaque) {
			alphaCut.push_back(1.0f);
		} else {
			material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutOff);
			alphaCut.push_back(alphaCutOff);
		}

		ai_real shininess;
		material->Get(AI_MATKEY_SHININESS, shininess);
		materialSpecularExponent.push_back(shininess);

		bool twoSided = false;
		material->Get(AI_MATKEY_TWOSIDED, twoSided);

		if (isOpaque) {
			if (!twoSided) {
				meshIndexes.push_back(meshIndex);
			} else {
				twoSidedMeshIndexes.push_back(meshIndex);
			}
		} else {
			if (!twoSided) {
				alphaCutMeshIndexes.push_back(meshIndex);
			} else {
				alphaCutTwoSidedMeshIndexes.push_back(meshIndex);
			}
		}

		//create the constant buffer and make space for the textures
		//crea el constant buffer y el espacio para las texturas
		ConstantsBufferViewData<LightingShaderConstants> cbvMeshData;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 5U, d3dDevice, cbvMeshData);
		if (numTextures > 0) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE diffuseCpuHandle(cbvCpuHandle);
			d3dDevice->CreateShaderResourceView(diffuseTexture.Get(), &diffuseSrvDesc, diffuseCpuHandle);
		}
		if (numTextures > 1) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE normalMapCpuHandle(cbvCpuHandle);
			normalMapCpuHandle.Offset(cbvMeshData.cbvDescriptorSize);
			d3dDevice->CreateShaderResourceView(normalMapTexture.Get(), &normalMapSrvDesc, normalMapCpuHandle);
		}

		//create the descriptors and shaders resource views for the shadowmaps
		//crea los descriptores y los shader resource views de los shadowmaps
		CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCpuHandle(cbvCpuHandle);
		directionalLightShadowMapCpuHandle.Offset(cbvMeshData.cbvDescriptorSize * 2);
		CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCpuHandle(cbvCpuHandle);
		spotLightShadowMapCpuHandle.Offset(cbvMeshData.cbvDescriptorSize * 3);
		CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCpuHandle(cbvCpuHandle);
		pointLightShadowMapCpuHandle.Offset(cbvMeshData.cbvDescriptorSize * 4);

		CreateShadowMapResourceView(d3dDevice, directionalLightShadowMap, directionalLightShadowMapCpuHandle);
		CreateShadowMapResourceView(d3dDevice, spotLightShadowMap, spotLightShadowMapCpuHandle);
		CreateShadowMapResourceView(d3dDevice, pointLightShadowMap, pointLightShadowMapCpuHandle);

		cbvData.push_back(cbvMeshData);
		hasNormalMap.push_back(numTextures > 1);

		ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants> directionalLightShadowMapMeshCbvData;
		CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCbvCpuHandle = InitializeConstantsBufferView(numFrames, 1U, d3dDevice, directionalLightShadowMapMeshCbvData);
		d3dDevice->CreateShaderResourceView(diffuseTexture.Get(), &diffuseSrvDesc, directionalLightShadowMapCbvCpuHandle);
		directionalLightShadowMapCbvData.push_back(directionalLightShadowMapMeshCbvData);

		ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants> spotLightShadowMapMeshCbvData;
		CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCbvCpuHandle = InitializeConstantsBufferView(numFrames, 1U, d3dDevice, spotLightShadowMapMeshCbvData);
		d3dDevice->CreateShaderResourceView(diffuseTexture.Get(), &diffuseSrvDesc, spotLightShadowMapCbvCpuHandle);
		spotLightShadowMapCbvData.push_back(spotLightShadowMapMeshCbvData);

		ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants> pointLightShadowMapMeshCbvData[6];
		for (UINT i = 0; i < 6; i++) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCbvCpuHandle = InitializeConstantsBufferView(numFrames, 1U, d3dDevice, pointLightShadowMapMeshCbvData[i]);
			d3dDevice->CreateShaderResourceView(diffuseTexture.Get(), &diffuseSrvDesc, pointLightShadowMapCbvCpuHandle);
			pointLightShadowMapCbvData[i].push_back(pointLightShadowMapMeshCbvData[i]);
		}
	}

	//no more need of the importer, so free the scene
	//no hay mas necesidad del importer, asi que liberamos la escena
	importer.FreeScene();

	//load the vertex/pixel shaders in async way
	//cargar el vertex/pixel shader de manera asincrona
	auto createVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\Static3DModel.cso").then([this](std::vector<byte>& fileData) {
		vertexShader = fileData;
	});
	auto createPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\Static3DModel.cso").then([this](std::vector<byte>& fileData) {
		pixelShader = fileData;
	});

	auto createPipelineStateTask = (createPSTask && createVSTask).then([this, d3dDevice]() {
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
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
		NAME_D3D12_OBJECT(rootSignature);

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,BiTangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexType,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//create the pipelineState for the opaque meshes
		//crea el pipelineState para los meshs opacos
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueState = {};
		InitializePipelineState(opaqueState, inputLayout, _countof(inputLayout), rootSignature, vertexShader, pixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&opaqueState, IID_PPV_ARGS(&pipelineState)));

		//create the pipeline for the opaque two sided meshes
		//crear el pipeline para las mallas opacas de dos lados
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(opaqueState);
		opaqueTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&opaqueTwoSidedState, IID_PPV_ARGS(&twoSidedPipelineState)));

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
	auto createShadowMapAlphaCutVSTask = DX::ReadDataAsync(L"Shaders\\VertexShaders\\ShadowMapAlphaCut.cso").then([this](std::vector<byte>& fileData) {
		shadowMapAlphaCutVertexShader = fileData;
	});
	auto createShadowMapAlphaCutPSTask = DX::ReadDataAsync(L"Shaders\\PixelShaders\\ShadowMapAlphaCut.cso").then([this](std::vector<byte>& fileData) {
		shadowMapAlphaCutPixelShader = fileData;
	});

	(createPipelineStateTask&& createShadowMapVSTask && createShadowMapPSTask && createShadowMapAlphaCutVSTask && createShadowMapAlphaCutPSTask).then([this, d3dDevice]() {
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
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&shadowMapRootSignature)));
		NAME_D3D12_OBJECT(shadowMapRootSignature);

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//initialize the pipeline state for the shadow map
		//inicializa el pipeline state para el shadow map
		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapOpaqueState = {};
		InitializeShadowMapPipelineState(shadowMapOpaqueState, inputLayout, _countof(inputLayout), shadowMapRootSignature.Get(), shadowMapVertexShader, shadowMapPixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapOpaqueState, IID_PPV_ARGS(&shadowMapPipelineState)));

		//initialize the pipeline state for the shadow map with two sides
		//inicializa el pipeline state para el shadow map con dos lados
		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapOpaqueTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(shadowMapOpaqueState);
		shadowMapOpaqueTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapOpaqueTwoSidedState, IID_PPV_ARGS(&shadowMapTwoSidedPipelineState)));

		shadowMapVertexShader.clear();
		shadowMapPixelShader.clear();
	}).then([this, d3dDevice]() {
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

		D3D12_STATIC_SAMPLER_DESC samplers[1];
		for (UINT i = 0; i < _countof(samplers); i++) {
			InitializeSampler(samplers[i], i);
		}

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(_countof(parameter), parameter, _countof(samplers), samplers, rootSignatureFlags);

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&shadowMapAlphaCutRootSignature)));
		NAME_D3D12_OBJECT(shadowMapAlphaCutRootSignature);

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexType,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexType,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//initialize the pipeline state for the shadow map with alpha testing(cut)
		//inicializa el pipeline state para el shadow map con alpha testing(corte)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapAlphaCutState = {};
		InitializeShadowMapPipelineState(shadowMapAlphaCutState, inputLayout, _countof(inputLayout), shadowMapAlphaCutRootSignature.Get(), shadowMapAlphaCutVertexShader, shadowMapAlphaCutPixelShader);
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapAlphaCutState, IID_PPV_ARGS(&shadowMapAlphaCutPipelineState)));

		//initialize the pipeline state for the shadow map with alpha testing(cut) and two sides
		//inicializa el pipeline state para el shadow map con alpha testing(corte) y dos lados
		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapAlphaCutTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(shadowMapAlphaCutState);
		shadowMapAlphaCutTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapAlphaCutTwoSidedState, IID_PPV_ARGS(&shadowMapAlphaCutTwoSidedPipelineState)));
		
		shadowMapAlphaCutVertexShader.clear();
		shadowMapAlphaCutPixelShader.clear();

		loadingComplete = true;
	});
}

void Static3DModel::DestroyUploadResources() {
	for (auto& vbv : vbvData) {
		vbv.vertexBufferUpload->Release();
	}
	for (auto& ibv : ibvData) {
		ibv.indexBufferUpload->Release();
	}
	for (auto& textureUpload : texturesUpload) {
		textureUpload && textureUpload->Release();
	}
}

void Static3DModel::Destroy() {
	for (auto& texture : textures) {
		texture->Release();
	}
	for (auto& vbv : vbvData) {
		vbv.vertexBuffer->Release();
	}
	for (auto& ibv : ibvData) {
		ibv.indexBuffer->Release();
	}
	for (auto& cbv : cbvData) {
		cbv.constantBuffer->Release();
		cbv.cbvsrvHeap->Release();
	}
	for (auto& cbv : directionalLightShadowMapCbvData) {
		cbv.constantBuffer->Release();
		cbv.cbvsrvHeap->Release();
	}
	for (auto& cbv : spotLightShadowMapCbvData) {
		cbv.constantBuffer->Release();
		cbv.cbvsrvHeap->Release();
	}
	for (auto& cbvVec : pointLightShadowMapCbvData) {
		for (auto& cbv : cbvVec) {
			cbv.constantBuffer->Release();
			cbv.cbvsrvHeap->Release();
		}
	}

	rootSignature->Release();
	pipelineState->Release();
	twoSidedPipelineState->Release();
	shadowMapRootSignature->Release();
	shadowMapPipelineState->Release();
	shadowMapTwoSidedPipelineState->Release();
	shadowMapAlphaCutRootSignature->Release();
	shadowMapAlphaCutPipelineState->Release();
	shadowMapAlphaCutTwoSidedPipelineState->Release();
}

void Static3DModel::UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled) {
	if (!loadingComplete) return;

	LightingShaderConstants constants;

	constants.numTextures = 1;
	constants.useBlinnPhong = useBlinnPhong;
	constants.normalMapTextureIndex = 1;
	constants.shadowMapsTextureIndex = 2;
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
	constants.directionalLightShadowMapZBias = 0.00001f;
	constants.spotLightShadowMapZBias = 0.00001f;
	constants.pointLightShadowMapZBias = 0.00001f;
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

	for (UINT meshIndex = 0; meshIndex < cbvData.size(); meshIndex++) {
		auto cbv = cbvData[meshIndex];
		constants.normalMaps = hasNormalMap[meshIndex] && normalMappingEnabled;
		constants.hasAlphaCut = alphaCut[meshIndex] != 1.0f;
		constants.alphaCut = alphaCut[meshIndex];
		constants.materialSpecularExponent = materialSpecularExponent[meshIndex];
		constants.shadowMaps = shadowMapsEnabled;
		UINT8* destination = cbv.mappedConstantBuffer + (backBufferIndex * cbv.alignedConstantBufferSize);
		memcpy(destination, &constants, sizeof(constants));
	}
}

void Static3DModel::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex) {
	if (!loadingComplete) return;
	
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	//render the one sided meshes
	//dibuja las primitivas de un solo lado
	commandList->SetPipelineState(pipelineState.Get());

	//render the non alpha tested meshes
	//dibuja las primitivas sin alpha test
	std::for_each(meshIndexes.begin(), meshIndexes.end(), [this,commandList, backBufferIndex](auto meshIndex) {
		Render3DMesh(commandList, backBufferIndex, *this, meshIndex);
	});

	//render the alpha tested meshes
	//dibuja las primitivas con alpha test
	std::for_each(alphaCutMeshIndexes.begin(), alphaCutMeshIndexes.end(), [this, commandList, backBufferIndex](auto meshIndex) {
		Render3DMesh(commandList, backBufferIndex, *this, meshIndex);
	});

	//render the two sided meshes
	//dibuja las primitivas de dos lados
	commandList->SetPipelineState(twoSidedPipelineState.Get());

	//render the non alpha tested meshes
	//dibuja las primitivas sin alpha test
	std::for_each(twoSidedMeshIndexes.begin(), twoSidedMeshIndexes.end(), [this, commandList, backBufferIndex](auto meshIndex) {
		Render3DMesh(commandList, backBufferIndex, *this, meshIndex);
	});

	//render the alpha tested meshes
	//dibuja las primitivas con alpha test
	std::for_each(alphaCutTwoSidedMeshIndexes.begin(), alphaCutTwoSidedMeshIndexes.end(), [this, commandList, backBufferIndex](auto meshIndex) {
		Render3DMesh(commandList, backBufferIndex, *this, meshIndex);
	});
}

void Static3DModel::UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>>& shadowMapCbvData) {
	if (!loadingComplete) return;

	ShadowMapAlphaCutShaderConstants constants;

	XMMATRIX shadowMapWorldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, shadowMapViewProjection));

	constants.worldViewProjection = shadowMapWorldViewProjection;

	for (UINT meshIndex = 0; meshIndex < shadowMapCbvData.size(); meshIndex++) {
		constants.alphaCut = alphaCut[meshIndex];
		UINT8* destination = shadowMapCbvData[meshIndex].mappedConstantBuffer + (backBufferIndex * shadowMapCbvData[meshIndex].alignedConstantBufferSize);
		memcpy(destination, &constants, sizeof(constants));
	}
}

void Static3DModel::RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, std::vector<ConstantsBufferViewData<ShadowMapAlphaCutShaderConstants>>& shadowMapCbvData) {
	if (!loadingComplete) return;

	//render the primitives without alpha test
	//dibuja las primitivas sin alpha test
	commandList->SetGraphicsRootSignature(shadowMapRootSignature.Get());

	//render the one sided meshes
	//dibuja las primitivas de un solo lado
	commandList->SetPipelineState(shadowMapPipelineState.Get());
	std::for_each(meshIndexes.begin(), meshIndexes.end(), [this, commandList, backBufferIndex, &shadowMapCbvData](auto meshIndex) {
		Render3DMeshShadowMap(commandList, backBufferIndex, shadowMapCbvData[meshIndex], *this, meshIndex);
	});

	//render the two sided meshes
	//dibuja las primitivas de dos lados
	commandList->SetPipelineState(shadowMapTwoSidedPipelineState.Get());
	std::for_each(twoSidedMeshIndexes.begin(), twoSidedMeshIndexes.end(), [this, commandList, backBufferIndex, &shadowMapCbvData](auto meshIndex) {
		Render3DMeshShadowMap(commandList, backBufferIndex, shadowMapCbvData[meshIndex], *this, meshIndex);
	});

	//render the primitives with alpha test
	//dibuja las primitivas con alpha test
	commandList->SetGraphicsRootSignature(shadowMapAlphaCutRootSignature.Get());

	//render the one sided meshes
	//dibuja las primitivas de un solo lado
	commandList->SetPipelineState(shadowMapAlphaCutPipelineState.Get());
	std::for_each(alphaCutMeshIndexes.begin(), alphaCutMeshIndexes.end(), [this, commandList, backBufferIndex, &shadowMapCbvData](auto meshIndex) {
		Render3DMeshShadowMap(commandList, backBufferIndex, shadowMapCbvData[meshIndex], *this, meshIndex, TRUE);
	});

	//render the two sided meshes
	//dibuja las primitivas de dos lados
	commandList->SetPipelineState(shadowMapAlphaCutTwoSidedPipelineState.Get());
	std::for_each(alphaCutTwoSidedMeshIndexes.begin(), alphaCutTwoSidedMeshIndexes.end(), [this, commandList, backBufferIndex, &shadowMapCbvData](auto meshIndex) {
		Render3DMeshShadowMap(commandList, backBufferIndex, shadowMapCbvData[meshIndex], *this, meshIndex, TRUE);
	});
}