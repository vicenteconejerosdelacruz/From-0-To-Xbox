cbuffer ConstantsBuffer : register(b0)
{
	matrix worldViewProjection;
	uint index;
	float alphaCut;
};