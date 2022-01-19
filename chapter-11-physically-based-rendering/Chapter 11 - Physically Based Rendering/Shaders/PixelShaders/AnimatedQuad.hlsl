#include "AnimatedQuadConstantsBuffer.h"

Texture2DArray tex : register(t0);
SamplerState samp0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color = 0.0f.xxxx;
	color += tex.Sample(samp0, float3(input.uv,index));
	if (color.a < alphaCut) {
		discard;
	}

	return float4(color.rgb,1.0f);
}
