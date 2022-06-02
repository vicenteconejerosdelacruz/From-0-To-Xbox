#include "SkinningPBRConstantsBuffer.h"
//#include "Lighting.h"
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

static const float M_PI = 3.141592653589793;

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 F_Schlick(float3 f0, float3 f90, float VdotH)
{
	return f0 + (f90 - f0) * pow(saturate(1.0 - VdotH), 5.0);
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float D_GGX(float NdotH, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;
	float f = (NdotH * NdotH) * (alphaRoughnessSq - 1.0) + 1.0;
	return alphaRoughnessSq / (M_PI * f * f);
}

// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if (GGX > 0.0)
	{
		return 0.5 / GGX;
	}
	return 0.0;
}

//https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 BRDF_lambertian(float3 f0, float3 f90, float3 diffuseColor, float VdotH)
{
	// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	return (1.0 - F_Schlick(f0, f90, VdotH)) * (diffuseColor / M_PI);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 BRDF_specularGGX(float3 f0, float3 f90, float alphaRoughness, float VdotH, float NdotL, float NdotV, float NdotH)
{
	float3 F = F_Schlick(f0, f90, VdotH);
	float Vis = V_GGX(NdotL, NdotV, alphaRoughness);
	float D = D_GGX(NdotH, alphaRoughness);

	return F * Vis * D;
}

void XXX(float3 viewDirection,float3 directionToLight,float3 normal,float3 f0, float3 f90, float3 albedoColor, float roughness, out float3 diffuseColor, out float3 specularColor) {

	float3 halfWay = normalize(directionToLight + viewDirection);
	float NdotL = saturate(dot(normal, directionToLight));
	float NdotV = saturate(dot(normal, viewDirection));
	float NdotH = saturate(dot(normal, halfWay));
	float LdotH = saturate(dot(directionToLight, halfWay));
	float VdotH = saturate(dot(viewDirection, halfWay));

	diffuseColor = 0.0f.xxx;
	specularColor = 0.0f.xxx;

	if (NdotL > 0.0 || NdotV > 0.0)
	{
		diffuseColor += NdotL * BRDF_lambertian(f0, f90, albedoColor, VdotH);
		specularColor += NdotL * BRDF_specularGGX(f0, f90, roughness * roughness, VdotH, NdotL, NdotV, NdotH);
	}
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 texturesColor = 0;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv);
	}

	if (hasAlphaCut && texturesColor.a < alphaCut) {
		discard;
	}

	float3 normal = normalize(input.normal);

	if (normalMaps) {
		float3 normalMap = texs[normalMapTextureIndex].Sample(samp0, input.uv).rgb*2.0f - 1.0f;
		float3 tangent = normalize(input.tangent);
		float3 biTangent = normalize(input.biTangent);

		float3x3 tangentToWorldSpace = float3x3(tangent, biTangent, normal);
		normal = normalize(mul(normalMap, tangentToWorldSpace));
	}

	float3 baseColor = texturesColor.rgb;

	//ambient occlusion, roughness and metalness
	float3 ARM = texs[normalMapTextureIndex + 1].Sample(samp0, input.uv).xyz;
	float ambientOcclusion = ARM.x;
	float roughness = ARM.y * roughnessFactor;
	float metallic = ARM.z * metallicFactor;

	float f0_ior = 0.04f;

	float3 f0 = float3(f0_ior.xxx);

	float3 albedoColor = lerp(baseColor * (1.0f.xxx - f0), 0.0f.xxx, metallic);
	f0 = lerp(f0, baseColor, metallic.xxx);

	float reflectance = max(max(f0.r, f0.g), f0.b);
	// Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
	float3 f90 = float3(saturate(reflectance * 50.0).xxx);

	float3 directionalDiffuseLightFactor;
	float3 directionalLightSpecularFactor;
	XXX(input.viewDirection, -directionalLightDirection.xyz, normal, f0, f90, albedoColor, roughness, directionalDiffuseLightFactor, directionalLightSpecularFactor);

	float3 spotLightDiffuseFactor;
	float3 spotLightSpecularFactor;
	XXX(input.viewDirection, normalize(spotLightPosition.xyz - input.worldPos).xyz, normal, f0, f90, albedoColor, roughness, spotLightDiffuseFactor, spotLightSpecularFactor);

	float3 pointLightDiffuseFactor;
	float3 pointLightSpecularFactor;
	XXX(input.viewDirection, normalize(pointLightPosition.xyz - input.worldPos).xyz, normal, f0, f90, albedoColor, roughness, pointLightDiffuseFactor, pointLightSpecularFactor);

	//calculate the directional light diffuse contribution
	//calcula la contribucion difusa de la luz direccional
	float3 directionalLightDiffuseContribution = directionalLightColor.rgb * directionalDiffuseLightFactor.rgb;

	//calculate the directional light specular contribution
	//calcula la contribucion difusa de la especular direccional
	float3 directionalLightSpecularContribution = directionalLightColor.rgb * directionalLightSpecularFactor.rgb;

	//calculate the spot light diffuse contribution
	//calcula la contribucion difusa de la luz spot
	float3 spotLightDiffuseContribution = spotLightColor.rgb * spotLightDiffuseFactor;

	//calculate the spot light diffuse contribution
	//calcula la contribucion difusa de la luz spot
	float3 spotLightSpecularContribution = spotLightColor.rgb * spotLightSpecularFactor;

	//calculate the point light diffuse contribution
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiffuseContribution = pointLightColor.rgb * pointLightDiffuseFactor;

	//calculate the point light specular contribution
	//calcula la contribucion especular de la luz omni-direcional
	float3 pointLightSpecularContribution = pointLightColor.rgb * pointLightSpecularFactor;

	//calculate the directional light shadow map factor
	//calcular el factor del shadow map de la luz direccional
	float directionalLightShadowMapFactor;
	if (shadowMaps) {
		float4 directionalLightShadowMapCoords = mul(float4(input.worldPos, 1.0f), directionalLightShadowMapProjection);
		calculateShadowMapFactor(directionalLightShadowMapCoords, texs[shadowMapsTextureIndex], samp0, directionalLightShadowMapZBias, directionalLightShadowMapTexelInvSize, directionalLightShadowMapFactor);
	}
	else {
		directionalLightShadowMapFactor = 1.0f;
	}

	//calculate the spot light shadow map factor
	//calcular el factor del shadow map de la luz spot
	float spotLightShadowMapFactor;
	if (shadowMaps) {
		float4 spotLightShadowMapCoords = mul(float4(input.worldPos, 1.0f), spotLightShadowMapProjection);
		calculateShadowMapFactor(spotLightShadowMapCoords, texs[shadowMapsTextureIndex + 1], samp0, spotLightShadowMapZBias, spotLightShadowMapTexelInvSize, spotLightShadowMapFactor);
	}
	else {
		spotLightShadowMapFactor = 1.0f;
	}

	//calculate the point light shadow map factor
	//calcular el factor del shadow map de la luz omni direccional
	float pointLightShadowMapFactor;
	if (shadowMaps) {
		calculatePointShadowMapFactor(input.worldPos, pointLightShadowMapProjection, texs[shadowMapsTextureIndex + 2], samp0, pointLightShadowMapZBias, pointLightShadowMapPartialDerivativeScale, pointLightShadowMapFactor);
	}
	else {
		pointLightShadowMapFactor = 1.0f;
	}

	float4 color = float4(
		directionalLightDiffuseContribution * directionalLightShadowMapFactor
		+
		directionalLightSpecularContribution * directionalLightShadowMapFactor
		+
		spotLightDiffuseContribution * spotLightShadowMapFactor
		+
		spotLightSpecularContribution * spotLightShadowMapFactor
		+
		pointLightDiffuseContribution * pointLightShadowMapFactor
		+
		pointLightSpecularContribution * pointLightShadowMapFactor
		, 1.0f);
	color.xyz = toGammaSpace(color.xyz);

	return color;
}