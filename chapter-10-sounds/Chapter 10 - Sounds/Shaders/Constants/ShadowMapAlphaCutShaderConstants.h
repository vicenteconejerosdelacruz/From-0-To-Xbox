#pragma once

struct ShadowMapAlphaCutShaderConstants {
  XMMATRIX worldViewProjection;
  FLOAT alphaCut;
};