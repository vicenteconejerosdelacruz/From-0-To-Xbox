cbuffer WorldViewProjectionConstantBuffer : register(b0)
{
	matrix wvp;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f),wvp);
	float3 color = input.color;

	output.pos = pos;
	output.color = color;

	return output;
}
