cbuffer WorldViewProjectionConstantBuffer : register(b0)
{
	matrix wvp;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f),wvp);
	float2 uv = input.uv;

	output.pos = pos;
	output.uv = uv;

	return output;
}
