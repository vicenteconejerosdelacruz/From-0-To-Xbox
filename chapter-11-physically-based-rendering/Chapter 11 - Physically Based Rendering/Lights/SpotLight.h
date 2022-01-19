#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct SpotLight {
	//lighting attributes
	//atributos de iluminacion
	XMVECTOR	color;
	XMVECTOR	position;
	XMFLOAT2	rotation;
	FLOAT			angle;
	XMVECTOR	attenuation;
	XMVECTOR directionAndAngle() {
		return {
			sinf(rotation.x) * cosf(rotation.y),
			sinf(rotation.y),
			cosf(rotation.x) * cosf(rotation.y),
			cosf(angle)
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

	void Initialize(XMVECTOR lightColor, XMVECTOR lightPosition, XMFLOAT2 lightRotation, FLOAT lightAngle, XMVECTOR lightAttenuation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT farZ);
};