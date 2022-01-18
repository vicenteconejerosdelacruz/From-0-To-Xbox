#include "LightingConstantsBuffer.h"

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal: NORMAL;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal: NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 worldPos : TEXCOORD2;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	float3 normal = mul(input.normal, (float3x3)world);
	float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
	float3 viewDirection = eyePos.xyz - worldPos;

	output.pos = pos;
	output.normal = normal;
	output.viewDirection = viewDirection;
	output.worldPos = worldPos;

	return output;
}
