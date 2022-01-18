#include "LightingConstantsBuffer.h"
#include "Lighting.h"
#include "ShadowMap.h"

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
SamplerState samp0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal: NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 worldPos : TEXCOORD2;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 texturesColor = 1.0f.xxxx;
	
	float2 coord = input.worldPos.xz;

	//grid based on
	//grilla basada de
	//http://madebyevan.com/shaders/grid/
	float2 grid1 = abs(frac(coord - 0.5f) - 0.5f) / fwidth(coord);
	float l1 = min(grid1.x, grid1.y);
	float unitColor = float(1.0f - min(l1, 1.0f));

	float2 grid2 = abs(frac((coord - 0.05f)*10.0f) - 0.5f) / fwidth(coord*10.0f);
	float l2 = min(grid2.x, grid2.y);
	float subUnitColor = float(1.0f - min(l2, 1.0f));

	texturesColor.rgb -= 0.5f * unitColor.xxx + 0.3f*subUnitColor.xxx;

	float3 normal = normalize(input.normal);
	float3 viewDir = normalize(input.viewDirection);

	float directionalLightDiffuseFactor;
	float directionalLightSpecularFactor;
	if (!useBlinnPhong) {
		calculateDirectionalLightFactorsPhong(normal, viewDir, directionalLightDirection.xyz, materialSpecularExponent, directionalLightDiffuseFactor, directionalLightSpecularFactor);
	} else {
		calculateDirectionalLightFactorsBlinnPhong(normal, viewDir, directionalLightDirection.xyz, materialSpecularExponent, directionalLightDiffuseFactor, directionalLightSpecularFactor);
	}

	float spotLightDiffuseFactor;
	float spotLightSpecularFactor;
	if (!useBlinnPhong) {
		calculateSpotLightFactorsPhong(input.worldPos, normal, viewDir, spotLightPosition.xyz, spotLightDirectionAndAngle.xyz, spotLightDirectionAndAngle.w, spotLightAttenuation.xyz, materialSpecularExponent, spotLightDiffuseFactor, spotLightSpecularFactor);
	} else {
		calculateSpotLightFactorsBlinnPhong(input.worldPos, normal, viewDir, spotLightPosition.xyz, spotLightDirectionAndAngle.xyz, spotLightDirectionAndAngle.w, spotLightAttenuation.xyz, materialSpecularExponent, spotLightDiffuseFactor, spotLightSpecularFactor);
	}

	float pointLightDiffuseFactor;
	float pointLightSpecularFactor;
	if (!useBlinnPhong) {
		calculatePointLightFactorsPhong(input.worldPos, normal, viewDir, pointLightPosition.xyz, pointLightAttenuation.xyz, materialSpecularExponent, pointLightDiffuseFactor, pointLightSpecularFactor);
	} else {
		calculatePointLightFactorsBlinnPhong(input.worldPos, normal, viewDir, pointLightPosition.xyz, pointLightAttenuation.xyz, materialSpecularExponent, pointLightDiffuseFactor, pointLightSpecularFactor);
	}

	//calculate the directional light diffuse contribution
	//calcula la contribucion difusa de la luz direccional
	float3 directionalLightDiffuseContribution = directionalLightColor.rgb * directionalLightDiffuseFactor;

	//calculate the directional light specular contribution
	//calcula la contribucion especular de la luz direccional
	float3 directionalLightSpecularContribution = directionalLightColor.rgb * directionalLightSpecularFactor;

	//calculate the spot light diffuse contribution
	//calcula la contribucion difusa de la luz spot
	float3 spotLightDiffuseContribution = spotLightColor.rgb * spotLightDiffuseFactor;

	//calculate the spot light specular contribution
	//calcula la contribucion especular de la luz spot
	float3 spotLightSpecularContribution = spotLightColor.rgb * spotLightSpecularFactor;

	//calculate the point light diffuse contribution
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiffuseContribution = pointLightColor.rgb * pointLightDiffuseFactor;

	//calculate the point light specular contribution
	//calcula la contribucion especular de la luz omni-direcional
	float3 pointLightSpecularContribution = pointLightColor.rgb * pointLightSpecularFactor;

	//copy the ambient light contribution
	//copiar la contribucion de la luz ambiente
	float3 ambientLightContribution = ambientLight.rgb;

	//calculate the directional light shadow map factor
	//calcular el factor del shadow map de la luz direccional
	float directionalLightShadowMapFactor;
	if (shadowMaps) {
		float4 directionalLightShadowMapCoords = mul(float4(input.worldPos, 1.0f), directionalLightShadowMapProjection);
		calculateShadowMapFactor(directionalLightShadowMapCoords, tex0, samp0, directionalLightShadowMapZBias, directionalLightShadowMapTexelInvSize, directionalLightShadowMapFactor);
	} else {
		directionalLightShadowMapFactor = 1.0f;
	}

	//calculate the spot light shadow map factor
	//calcular el factor del shadow map de la luz spot
	float spotLightShadowMapFactor;
	if (shadowMaps) {
		float4 spotLightShadowMapCoords = mul(float4(input.worldPos, 1.0f), spotLightShadowMapProjection);
		calculateShadowMapFactor(spotLightShadowMapCoords, tex1, samp0, spotLightShadowMapZBias, spotLightShadowMapTexelInvSize, spotLightShadowMapFactor);
	} else {
		spotLightShadowMapFactor = 1.0f;
	}

	//calculate the point light shadow map factor
	//calcular el factor del shadow map de la luz omni direccional
	float pointLightShadowMapFactor;
	if (shadowMaps) {
		calculatePointShadowMapFactor(input.worldPos, pointLightShadowMapProjection, tex2, samp0, pointLightShadowMapZBias, pointLightShadowMapPartialDerivativeScale, pointLightShadowMapFactor);
	} else {
		pointLightShadowMapFactor = 1.0f;
	}

	//create the light final color
	//crear el color final de la luz
	float4 color = float4(
		directionalLightDiffuseContribution * directionalLightShadowMapFactor
		+ 
		spotLightDiffuseContribution * spotLightShadowMapFactor
		+
		pointLightDiffuseContribution * pointLightShadowMapFactor
		+
		ambientLightContribution
	,1.0f);

	//add the textures contribution
	//agrega la contribucion de las texturas
	color.rgb = saturate(color.rgb) * texturesColor.rgb;
	color.rgb += directionalLightSpecularContribution * directionalLightShadowMapFactor;
	color.rgb += spotLightSpecularContribution * spotLightShadowMapFactor;
	color.rgb += pointLightSpecularContribution * pointLightShadowMapFactor;

	return color;
}