#include "SkinningConstantsBuffer.h"

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 biTangent: BITANGENT;
	float2 uv : TEXCOORD0;
	uint4 boneIds : BLENDINDICES;
	float4 boneWeights : BLENDWEIGHT;
};

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

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	matrix boneTransform = boneMatrices[input.boneIds.x] * input.boneWeights.x +
		boneMatrices[input.boneIds.y] * input.boneWeights.y +
		boneMatrices[input.boneIds.z] * input.boneWeights.z +
		boneMatrices[input.boneIds.w] * input.boneWeights.w;

	float3 skinnedPos = mul(float4(input.pos, 1.0f), boneTransform).xyz;
	float4 pos = mul(float4(skinnedPos, 1.0f), worldViewProjection);
	float3 skinnedNormal = mul(input.normal, (float3x3)boneTransform);
	float3 normal = mul(skinnedNormal, (float3x3)world);
	float3 skinnedTangent = mul(input.tangent, (float3x3)boneTransform);
	float3 tangent = mul(skinnedTangent, (float3x3)world);
	float3 skinnedBiTangent = mul(input.biTangent, (float3x3)boneTransform);
	float3 biTangent = mul(skinnedBiTangent, (float3x3)world);
	float3 worldPos = mul(float4(skinnedPos, 1.0f), world).xyz;
	float3 viewDirection = eyePos.xyz - worldPos;
	float2 uv = input.uv;

	output.pos = pos;
	output.normal = normal;
	output.tangent = tangent;
	output.biTangent = biTangent;
	output.uv = uv;
	output.viewDirection = viewDirection;
	output.worldPos = worldPos;

	return output;
}