#include "pch.h"
#include "DirectionalLight.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void DirectionalLight::Initialize(XMVECTOR lightColor, XMFLOAT2 lightRotation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT viewWidth, FLOAT viewHeight, FLOAT farZ) {
	color = lightColor;
	rotation = lightRotation;

	//create the descriptor for the shadow and assign the depth-stencil buffer to it
	//creamos el descriptor para el shadow map y le asignamos el buffer del depth-stencil
	shadowMapDSVDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	NAME_D3D12_OBJECT(shadowMapDSVDescriptorHeap);
	UpdateDepthStencilView(d3dDevice, shadowMapDSVDescriptorHeap, shadowMap, shadowMapWidth, shadowMapHeight);

	shadowMapScissorRect = { 0, 0, static_cast<LONG>(shadowMapWidth), static_cast<LONG>(shadowMapHeight) };
	shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(shadowMapWidth), static_cast<FLOAT>(shadowMapHeight), 0.0f, 1.0f };
	shadowMapPerspectiveMatrix = XMMatrixOrthographicRH(viewWidth, viewHeight, 0.01f, farZ);
	shadowMapTexelInvSize = { 1.0f / static_cast<FLOAT>(shadowMapWidth), 1.0f / static_cast<FLOAT>(shadowMapHeight) };
}