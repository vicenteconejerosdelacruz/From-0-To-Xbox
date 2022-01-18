#pragma once

struct AnimatedQuadShaderConstants {
  XMMATRIX worldViewProjection;
  UINT index;
  FLOAT alphaCut;
};