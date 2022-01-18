cbuffer ConstantsBuffer : register(b0)
{
	uint numTextures;
	bool useBlinnPhong;
	float materialSpecularExponent;
	matrix worldViewProjection;
	matrix world;
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
};