#include "LightingConstantsBuffer.h"
#include "Lighting.h"
#include "ShadowMap.h"
#include "Gamma.h"

Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal: NORMAL;
	float2 uv : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
	float3 worldPos : TEXCOORD2;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 texturesColor = 0.0f.xxx;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv).rgb;
	}

	float3 normal = normalize(input.normal);
	float3 viewDir = normalize(input.viewDirection);

	float directionalDiffuseLightFactor;
	float directionalLightSpecularFactor;
	if (!useBlinnPhong) {
		calculateDirectionalLightFactorsPhong(normal, viewDir, directionalLightDirection.xyz, materialSpecularExponent, directionalDiffuseLightFactor, directionalLightSpecularFactor);
	} else {
		calculateDirectionalLightFactorsBlinnPhong(normal, viewDir, directionalLightDirection.xyz, materialSpecularExponent, directionalDiffuseLightFactor, directionalLightSpecularFactor);
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
	float3 directionalLightDiffuseContribution = directionalLightColor.rgb * directionalDiffuseLightFactor;

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
		calculateShadowMapFactor(directionalLightShadowMapCoords, texs[shadowMapsTextureIndex], samp0, directionalLightShadowMapZBias, directionalLightShadowMapTexelInvSize, directionalLightShadowMapFactor);
	} else {
		directionalLightShadowMapFactor = 1.0f;
	}

	//calculate the spot light shadow map factor
	//calcular el factor del shadow map de la luz spot
	float spotLightShadowMapFactor;
	if (shadowMaps) {
		float4 spotLightShadowMapCoords = mul(float4(input.worldPos, 1.0f), spotLightShadowMapProjection);
		calculateShadowMapFactor(spotLightShadowMapCoords, texs[shadowMapsTextureIndex + 1], samp0, spotLightShadowMapZBias, spotLightShadowMapTexelInvSize, spotLightShadowMapFactor);
	} else {
		spotLightShadowMapFactor = 1.0f;
	}

	//calculate the point light shadow map factor
	//calcular el factor del shadow map de la luz omni direccional
	float pointLightShadowMapFactor;
	if (shadowMaps) {
		calculatePointShadowMapFactor(input.worldPos, pointLightShadowMapProjection, texs[shadowMapsTextureIndex + 2], samp0, pointLightShadowMapZBias, pointLightShadowMapPartialDerivativeScale, pointLightShadowMapFactor);
	} else {
		pointLightShadowMapFactor = 1.0f;
	}
	
	//create the light final diffuse color
	//crear el color difuso final de la luz
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

	//add the specular contribution
	//agrega la contribucion de la luz especular
	color.rgb += directionalLightSpecularContribution * directionalLightShadowMapFactor;
	color.rgb += spotLightSpecularContribution * spotLightShadowMapFactor;
	color.rgb += pointLightSpecularContribution * pointLightShadowMapFactor;

	//return to gamma space
	//volvemos a espacio gamma
	return float4(toGammaSpace(color.rgb), 1.0f);
}