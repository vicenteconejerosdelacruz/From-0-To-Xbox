#include "SkinningShadowMapAlphaCutConstantsBuffer.h"

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	uint4 boneIds : BLENDINDICES;
	float4 boneWeights : BLENDWEIGHT;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	matrix boneTransform = boneMatrices[input.boneIds.x] * input.boneWeights.x +
		boneMatrices[input.boneIds.y] * input.boneWeights.y +
		boneMatrices[input.boneIds.z] * input.boneWeights.z +
		boneMatrices[input.boneIds.w] * input.boneWeights.w;

	float3 skinnedPos = mul(float4(input.pos, 1.0f), boneTransform).xyz;
	float4 pos = mul(float4(skinnedPos, 1.0f), worldViewProjection);

	float2 uv = input.uv;

	output.pos = pos;
	output.uv = uv;

	return output;
}
