#include "SkinningConstantsBuffer.h"
#include "Lighting.h"
#include "ShadowMap.h"
#include "Gamma.h"

Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float3 biTangent: BITANGENT;
	float2 uv : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
	float3 worldPos : TEXCOORD2;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 texturesColor = 0.0f.xxxx;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv);
	}

	if (hasAlphaCut && texturesColor.a < alphaCut) {
		discard;
	}

	float3 normal = normalize(input.normal);

	if (normalMaps) {
		float3 normalMap = texs[normalMapTextureIndex].Sample(samp0, input.uv).xyz*2.0f - 1.0f;
		float3 tangent = normalize(input.tangent);
		float3 biTangent = normalize(input.biTangent);

		float3x3 tangentToWorldSpace = float3x3(tangent, biTangent, normal);
		normal = normalize(mul(normalMap, tangentToWorldSpace));
	}

	float3 viewDir = normalize(input.viewDirection);

	float directionalLightDiffuseFactor;
	float directionalLightSpecularFactor;
	//just use phong as we are not dealing with specularity
	//solo usamos phong ya que no estamos manejando la especularidad
	calculateDirectionalLightFactorsPhong(normal, viewDir, directionalLightDirection.xyz, 0.0f, directionalLightDiffuseFactor, directionalLightSpecularFactor);

	float spotLightDiffuseFactor;
	float spotLightSpecularFactor;
	//just use phong as we are not dealing with specularity
	//solo usamos phong ya que no estamos manejando la especularidad
	calculateSpotLightFactorsPhong(input.worldPos, normal, viewDir, spotLightPosition.xyz, spotLightDirectionAndAngle.xyz, spotLightDirectionAndAngle.w, spotLightAttenuation.xyz, 0.0f, spotLightDiffuseFactor, spotLightSpecularFactor);

	float pointLightDiffuseFactor;
	float pointLightSpecularFactor;
	//just use phong as we are not dealing with specularity
	//solo usamos phong ya que no estamos manejando la especularidad
	calculatePointLightFactorsPhong(input.worldPos, normal, viewDir, pointLightPosition.xyz, pointLightAttenuation.xyz, 0.0f, pointLightDiffuseFactor, pointLightSpecularFactor);

	//calculate the directional light diffuse contribution
	//calcula la contribucion difusa de la luz direccional
	float3 directionalLightDiffuseContribution = directionalLightColor.rgb * directionalLightDiffuseFactor;

	//calculate the spot light diffuse contribution
	//calcula la contribucion difusa de la luz spot
	float3 spotLightDiffuseContribution = spotLightColor.rgb * spotLightDiffuseFactor;

	//calculate the point light diffuse contribution
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiffuseContribution = pointLightColor.rgb * pointLightDiffuseFactor;

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

	//return to gamma space
	//volvemos a espacio gamma
	return float4(toGammaSpace(color.rgb), 1.0f);
}