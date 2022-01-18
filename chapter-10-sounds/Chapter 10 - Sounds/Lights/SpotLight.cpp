#include "pch.h"
#include "SpotLight.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"

void SpotLight::Initialize(XMVECTOR lightColor, XMVECTOR lightPosition, XMFLOAT2 lightRotation, FLOAT lightAngle, XMVECTOR lightAttenuation, ComPtr<ID3D12Device2> d3dDevice, UINT shadowMapWidth, UINT shadowMapHeight, FLOAT farZ) {
	color = lightColor;
	position = lightPosition;
	rotation = lightRotation;
	angle = lightAngle;
	attenuation = lightAttenuation;

	//create the descriptor for the shadow and assign the depth-stencil buffer to it
	//creamos el descriptor para el shadow map y le asignamos el buffer del depth-stencil
	shadowMapDSVDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	NAME_D3D12_OBJECT(shadowMapDSVDescriptorHeap);
	UpdateDepthStencilView(d3dDevice, shadowMapDSVDescriptorHeap, shadowMap, shadowMapWidth, shadowMapHeight);

	shadowMapScissorRect = { 0, 0, static_cast<LONG>(shadowMapWidth), static_cast<LONG>(shadowMapHeight) };
	shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(shadowMapWidth), static_cast<FLOAT>(shadowMapHeight), 0.0f, 1.0f };
	shadowMapPerspectiveMatrix = XMMatrixPerspectiveFovRH(angle*2.0f, 1.0f , 0.01f, farZ);
	shadowMapTexelInvSize = { 1.0f/static_cast<FLOAT>(shadowMapWidth), 1.0f/static_cast<FLOAT>(shadowMapHeight) };
}