#pragma once

#define MAX_BONES 256

struct SkinningShaderConstants {
  UINT numTextures;
  BOOL normalMaps;
  UINT normalMapTextureIndex;
  BOOL shadowMaps;
  UINT shadowMapsTextureIndex;
  BOOL hasAlphaCut;
  FLOAT alphaCut;
  XMMATRIX worldViewProjection;
  XMMATRIX world;
  XMMATRIX directionalLightShadowMapProjection;
  XMMATRIX spotLightShadowMapProjection;
  XMMATRIX pointLightShadowMapProjection[6];
  FLOAT directionalLightShadowMapZBias;
  XMFLOAT2 directionalLightShadowMapTexelInvSize;
  FLOAT spotLightShadowMapZBias;
  XMFLOAT2 spotLightShadowMapTexelInvSize;
  FLOAT pointLightShadowMapZBias;
  FLOAT pointLightShadowMapPartialDerivativeScale;
  XMVECTOR eyePos;
  XMVECTOR ambientLightColor;
  XMVECTOR directionalLightDirection;
  XMVECTOR directionalLightColor;
  XMVECTOR spotLightPosition;
  XMVECTOR spotLightColor;
  XMVECTOR spotLightDirectionAndAngle;
  XMVECTOR spotLightAttenuation;
  XMVECTOR pointLightColor;
  XMVECTOR pointLightPosition;
  XMVECTOR pointLightAttenuation;
  XMMATRIX boneMatrices[MAX_BONES];
};