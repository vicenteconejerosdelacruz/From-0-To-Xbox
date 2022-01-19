static const int MAX_BONES = 256;

cbuffer ConstantsBuffer : register(b0)
{
	matrix worldViewProjection;
	float alphaCut;
	matrix boneMatrices[MAX_BONES];
};