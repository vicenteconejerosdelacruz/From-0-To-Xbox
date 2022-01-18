void calculateShadowMapFactor(float4 shadowMapCoords, Texture2D shadowMap, SamplerState samp, float shadowMapBias, float2 shadowMapInvTexelSize, out float shadowMapFactor) {
	shadowMapCoords.x = 0.5f + 0.5f * shadowMapCoords.x / shadowMapCoords.w;
	shadowMapCoords.y = 0.5f - 0.5f * shadowMapCoords.y / shadowMapCoords.w;
	shadowMapCoords.z /= shadowMapCoords.w;
	float shadowMapDepth[4];
	shadowMapDepth[0] = shadowMap.Sample(samp, shadowMapCoords.xy).r;
	shadowMapDepth[1] = shadowMap.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.x, 0.0f)).r;
	shadowMapDepth[2] = shadowMap.Sample(samp, shadowMapCoords.xy + float2(0.0f, shadowMapInvTexelSize.y)).r;
	shadowMapDepth[3] = shadowMap.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.xy)).r;
	shadowMapFactor = 1.0f;
	for (int i = 0; i < 4; i++) {
		if ((shadowMapDepth[i] + shadowMapBias) < shadowMapCoords.z) {
			shadowMapFactor -= 0.25f;
		}
	}
}

void getPointShadowMapFactor(float3 worldPos, matrix shadowMapProjection[6], Texture2D shadowMap, SamplerState samp, float shadowMapBias, out float shadowMapFactor) {
	shadowMapFactor = 1.0f;
	for (int i = 0; i < 6; i++) {
		float4 shadowMapCoords = mul(float4(worldPos, 1.0f), shadowMapProjection[i]);
		shadowMapCoords.xyz /= shadowMapCoords.w;
		//check if projected coords are in boundary for this shadow map projection
		if (shadowMapCoords.x > -1.0f && shadowMapCoords.x <  1.0f && shadowMapCoords.y > -1.0f && shadowMapCoords.y < 1.0f && shadowMapCoords.z > 0.0f && shadowMapCoords.z < 1.0f) {
			shadowMapCoords.x = 0.5f + 0.5f * shadowMapCoords.x;
			shadowMapCoords.y = 1.0f / 6.0f * i + (0.5f - 0.5f * shadowMapCoords.y) * 1.0f / 6.0f;
			float shadowMapDepth = shadowMap.Sample(samp, shadowMapCoords.xy).r;
			if ((shadowMapDepth + shadowMapBias) < shadowMapCoords.z) {
				shadowMapFactor = 0.0f;
				break;
			}
		}
	}
}

void calculatePointShadowMapFactor(float3 worldPos, matrix shadowMapProjection[6], Texture2D shadowMap, SamplerState samp, float shadowMapBias, float shadowMapPartialDerivativeScale, out float shadowMapFactor) {
	float3 worldPosDdx = ddx(worldPos) * shadowMapPartialDerivativeScale;
	float3 worldPosDdy = ddy(worldPos) * shadowMapPartialDerivativeScale;
	float4 shadowMapFactors;
	getPointShadowMapFactor(worldPos, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.x);
	getPointShadowMapFactor(worldPos + worldPosDdx, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.y);
	getPointShadowMapFactor(worldPos + worldPosDdy, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.z);
	getPointShadowMapFactor(worldPos + worldPosDdx + worldPosDdy, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.w);
	shadowMapFactor = dot(float(0.25f).xxxx, shadowMapFactors);
}
