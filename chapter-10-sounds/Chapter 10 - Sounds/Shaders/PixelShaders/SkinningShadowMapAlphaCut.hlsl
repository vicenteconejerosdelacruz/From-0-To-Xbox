#include "SkinningShadowMapAlphaCutConstantsBuffer.h"

Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

void main(PixelShaderInput input) : SV_TARGET
{
	float4 texturesColor = texs[0].Sample(samp0, input.uv);

	if (texturesColor.w < alphaCut) {
		discard;
	}
}
