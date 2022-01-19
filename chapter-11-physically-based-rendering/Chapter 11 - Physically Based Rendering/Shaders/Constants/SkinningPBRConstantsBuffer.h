static const int MAX_BONES = 256;

cbuffer ConstantsBuffer : register(b0)
{
	uint numTextures;
	bool normalMaps;
	uint normalMapTextureIndex;
	bool shadowMaps;
	uint shadowMapsTextureIndex;
	bool hasAlphaCut;
	float alphaCut;
	float metallicFactor;
	float roughnessFactor;
	matrix worldViewProjection;
	matrix world;
	matrix directionalLightShadowMapProjection;
	matrix spotLightShadowMapProjection;
	matrix pointLightShadowMapProjection[6];
	float directionalLightShadowMapZBias;
	float2 directionalLightShadowMapTexelInvSize;
	float spotLightShadowMapZBias;
	float2 spotLightShadowMapTexelInvSize;
	float pointLightShadowMapZBias;
	float pointLightShadowMapPartialDerivativeScale;
	float4 eyePos;
	float4 ambientLight;
	float4 directionalLightDirection;
	float4 directionalLightColor;
	float4 spotLightPosition;
	float4 spotLightColor;
	float4 spotLightDirectionAndAngle;
	float4 spotLightAttenuation;
	float4 pointLightColor;
	float4 pointLightPosition;
	float4 pointLightAttenuation;
	matrix boneMatrices[MAX_BONES];
};