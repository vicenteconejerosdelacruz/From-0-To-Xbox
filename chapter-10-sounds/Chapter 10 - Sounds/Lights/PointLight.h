#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct PointLight {
	XMVECTOR color;
	XMVECTOR position;
	XMVECTOR attenuation;

	static constexpr XMVECTOR direction[] = {
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f,-1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{-1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f,-1.0f, 0.0f },
	};

	static constexpr XMVECTOR up[] = {
		{ 0.0f, 0.0f,-1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
	};

	//shadow map resources
	//recursos del shadow map
	ComPtr<ID3D12Resource>				shadowMap;
	ComPtr<ID3D12DescriptorHeap>	shadowMapDSVDescriptorHeap;
	D3D12_RECT										shadowMapClearScissorRect;
	D3D12_VIEWPORT								shadowMapClearViewport;
	D3D12_RECT										shadowMapScissorRect[6];
	D3D12_VIEWPORT								shadowMapViewport[6];
	XMMATRIX											shadowMapPerspectiveMatrix;

	void Initialize(XMVECTOR lightColor, XMVECTOR lightPosition, XMVECTOR lightAttenuation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT farZ);
};