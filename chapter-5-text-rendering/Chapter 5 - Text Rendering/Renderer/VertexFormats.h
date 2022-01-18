#pragma once
using namespace DirectX;

struct VertexPosColor
{
  XMFLOAT3 Position;
  XMFLOAT3 Color;
};

struct VertexPosTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT2 TexCoord;
};

struct VertexPosNormalTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT2 TexCoord;
};