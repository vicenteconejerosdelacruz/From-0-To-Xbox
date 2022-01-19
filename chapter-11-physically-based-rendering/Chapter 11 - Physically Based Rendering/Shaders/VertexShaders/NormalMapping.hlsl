#include "LightingConstantsBuffer.h"

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float2 uv : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float2 uv : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
	float3 worldPos : TEXCOORD2;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	float3 normal = mul(input.normal, (float3x3)world);
	float3 tangent = mul(input.tangent, (float3x3)world);
	float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
	float3 viewDirection = eyePos.xyz - worldPos;
	float2 uv = input.uv;

	output.pos = pos;
	output.normal = normal;
	output.tangent = tangent;
	output.uv = uv;
	output.viewDirection = viewDirection;
	output.worldPos = worldPos;

	return output;
}