Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

static const uint numTextures = 2;
static const float GAMMA = 2.2f;
static const float INVGAMMA = 1.0f / GAMMA;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 texturesColor = 0.0f.xxx;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv).rgb;
	}

	return float4(pow(texturesColor.rgb, INVGAMMA ), 1.0f);
}
