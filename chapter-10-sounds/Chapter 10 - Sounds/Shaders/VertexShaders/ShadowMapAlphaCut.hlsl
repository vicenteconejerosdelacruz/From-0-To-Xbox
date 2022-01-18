matrix worldViewProjection : register(b0);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	float2 uv = input.uv;

	output.pos = pos;
	output.uv = uv;

	return output;
}
