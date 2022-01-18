#include "LightingConstantsBuffer.h"

Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

static const float GAMMA = 2.2f;
static const float INVGAMMA = 1.0f / GAMMA;
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

	//calculate the diffuse contribution from the directional light
	//calcular la contribucion difusa de la luz direccional
	float3 directionalLightVector = -normalize(directionalLightDirection.xyz);
	float directionalDiffuseLightFactor = saturate(dot(normal, directionalLightVector));

	//calculate the specular contribution from the directional light
	//calcular la contribucion especular de la luz direcional
	float directionalLightSpecularFactor = 0.0f;
	if (!useBlinnPhong) {
		//phong
		float3 directionalLightReflect = normalize(reflect(-directionalLightVector, normal));
		float viewDotDirectionalLight = dot(viewDir, directionalLightReflect);
		directionalLightSpecularFactor = pow(saturate(viewDotDirectionalLight), materialSpecularExponent);
	} else {
		//blinn-phong
		float3 halfWayDirectionalLight = normalize(directionalLightVector + viewDir);
		float normalDotHalfWayDirectional = dot(normal, halfWayDirectionalLight);
		directionalLightSpecularFactor = pow(saturate(normalDotHalfWayDirectional), materialSpecularExponent);
	}
	
	//calculate the diffuse contribution from the spot light
	//calcular la contribucion difusa de la luz spot
	float3 spotLightDiff = input.worldPos - spotLightPosition.xyz;
	float3 spotLightVector = normalize(spotLightDiff);
	float3 spotLightDirectionVector = normalize(spotLightDirectionAndAngle.xyz);
	float spotLightDiffuseFactor = dot(spotLightVector, spotLightDirectionVector);
	
	//declare spot light specular contribution
	//declarar la contribucion de la luz especular
	float spotLightSpecularFactor = 0;
	
	//if its inside the spot area
	//si es que estamos dentro del area de spot
	if (spotLightDiffuseFactor > spotLightDirectionAndAngle.w) {
		//calculate the spot light diffuse contribution
		//calcular la contribucion difusa de la luz spot
		spotLightDiffuseFactor = 1 - (1 - spotLightDiffuseFactor) / (1 - spotLightDirectionAndAngle.w);
		float spotLightDistance = length(spotLightDiff);
		float spotLightDistanceAttenuation = spotLightAttenuation.x + spotLightDistance * spotLightAttenuation.y + spotLightDistance * spotLightDistance * spotLightAttenuation.z;
		spotLightDiffuseFactor *= saturate(dot(normal, -spotLightVector));
		spotLightDiffuseFactor = spotLightDiffuseFactor / spotLightDistanceAttenuation;

		//calculate the spot light specular contribution
		//calcular la contribucion specular de la luz spot
		if (!useBlinnPhong) {
			//phong
			float3 spotLightReflect = normalize(reflect(spotLightVector, normal));
			float viewDotSpotLight = dot(viewDir, spotLightReflect);
			spotLightSpecularFactor = pow(saturate(viewDotSpotLight), materialSpecularExponent);
		} else {
			//blinn-phong
			float3 halfWaySpotLight = normalize(-spotLightVector + viewDir);
			float normalDotHalfWaySpot = dot(normal, halfWaySpotLight);
			spotLightSpecularFactor = pow(saturate(normalDotHalfWaySpot), materialSpecularExponent);
		}
	} else {
		spotLightDiffuseFactor = 0.0f;
	}
	
	//calculate the diffuse contribution from the point light
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiff = input.worldPos - pointLightPosition.xyz;
	float pointLightDistance = length(pointLightDiff);
	float pointLightDistanceAttenuation = pointLightAttenuation.x + pointLightDistance * pointLightAttenuation.y + pointLightDistance * pointLightDistance * pointLightAttenuation.z;
	float pointLightDiffuseFactor = saturate(dot(normal, -normalize(pointLightDiff))) / pointLightDistanceAttenuation;
	
	//calculate the specular contribution from the point light
	//calcula la contribucion especular de la luz omni-direcional
	float pointLightSpecularFactor = 0.0f;
	if (!useBlinnPhong) {
		//phong
		float3 pointLightReflect = normalize(reflect(pointLightDiff, normal));
		float viewDotPointLight = dot(viewDir, pointLightReflect);
		pointLightSpecularFactor = pow(saturate(viewDotPointLight), materialSpecularExponent);
	} else {
		//blinn-phong
		float3 halfWayPointLight = normalize(-pointLightDiff + viewDir);
		float normalDotHalfWayPoint = dot(normal, halfWayPointLight);
		pointLightSpecularFactor = pow(saturate(normalDotHalfWayPoint), materialSpecularExponent);
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

	//create the light final diffuse color
	//crear el color difuso final de la luz
	float4 color = float4(
		directionalLightDiffuseContribution 
		+ 
		spotLightDiffuseContribution 
		+
		pointLightDiffuseContribution
		+
		ambientLightContribution
	,1.0f);

	//add the textures contribution
	//agrega la contribucion de las texturas
	color.rgb = saturate(color.rgb) * texturesColor.rgb;

	//add the specular contribution
	//agrega la contribucion de la luz especular
	color.rgb += directionalLightSpecularContribution;
	color.rgb += spotLightSpecularContribution;
	color.rgb += pointLightSpecularContribution;

	//return to gamma space
	//volvemos a espacio gamma
	return float4(pow(color.rgb, INVGAMMA ), 1.0f);
}
