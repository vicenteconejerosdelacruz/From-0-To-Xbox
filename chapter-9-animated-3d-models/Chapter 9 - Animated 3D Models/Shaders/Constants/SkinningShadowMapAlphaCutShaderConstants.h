#pragma once

#define MAX_BONES 256

struct SkinningShadowMapAlphaCutShaderConstants {
  XMMATRIX worldViewProjection;
  FLOAT alphaCut;
  XMMATRIX boneMatrices[MAX_BONES];
};