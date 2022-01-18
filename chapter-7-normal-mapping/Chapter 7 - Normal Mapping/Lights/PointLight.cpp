#include "pch.h"
#include "PointLight.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void PointLight::Initialize(XMVECTOR lightColor, XMVECTOR lightPosition, XMVECTOR lightAttenuation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT farZ) {
	color = lightColor;
	position = lightPosition;
	attenuation = lightAttenuation;

	//create the descriptor for the shadow and assign the depth-stencil buffer to it
	//creamos el descriptor para el shadow map y le asignamos el buffer del depth-stencil
	shadowMapDSVDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	NAME_D3D12_OBJECT(shadowMapDSVDescriptorHeap);
	UpdateDepthStencilView(d3dDevice, shadowMapDSVDescriptorHeap, shadowMap, shadowMapWidth, shadowMapHeight*6U);

	for (UINT i = 0U; i < 6U; i++) {
		shadowMapScissorRect[i] = { 0, static_cast<LONG>(i) * static_cast<LONG>(shadowMapHeight), static_cast<LONG>(shadowMapWidth), static_cast<LONG>(i+1U) * static_cast<LONG>(shadowMapHeight) };
		shadowMapViewport[i] = { 0.0f, static_cast<FLOAT>(i) * static_cast<FLOAT>(shadowMapHeight), static_cast<FLOAT>(shadowMapWidth), static_cast<FLOAT>(shadowMapHeight), 0.0f, 1.0f };
	}
	shadowMapClearScissorRect = { 0, 0, static_cast<LONG>(shadowMapWidth), 6L * static_cast<LONG>(shadowMapHeight) };
	shadowMapClearViewport = { 0.0f, 0.0f , static_cast<FLOAT>(shadowMapWidth), 6L * static_cast<FLOAT>(shadowMapHeight), 0.0f, 1.0f };

	shadowMapPerspectiveMatrix = XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV2, 1.0f, 0.01f, farZ);
}