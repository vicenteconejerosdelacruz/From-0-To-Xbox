static const float GAMMA = 2.2f;
static const float INVGAMMA = 1.0f / GAMMA;

float3 toGammaSpace(float3 color) {
	return pow(abs(color), INVGAMMA);
}