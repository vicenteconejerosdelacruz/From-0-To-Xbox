#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct DirectionalLight {
	//lighting attributes
	//atributos de iluminacion
	XMVECTOR color;
	XMFLOAT2 rotation;
	XMVECTOR direction() {
		return {
			sinf(rotation.x) * cosf(rotation.y),
			sinf(rotation.y),
			cosf(rotation.x) * cosf(rotation.y)
		};
	}

	//shadow map resources
	//recursos del shadow map
	ComPtr<ID3D12Resource>				shadowMap;
	ComPtr<ID3D12DescriptorHeap>	shadowMapDSVDescriptorHeap;
	D3D12_RECT										shadowMapScissorRect;
	D3D12_VIEWPORT								shadowMapViewport;
	XMMATRIX											shadowMapPerspectiveMatrix;
	XMFLOAT2											shadowMapTexelInvSize;

	void Initialize(XMVECTOR lightColor, XMFLOAT2 lightRotation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT viewWidth, FLOAT viewHeight, FLOAT farZ);
};