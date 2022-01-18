#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct AmbientLight {
	XMVECTOR color;

	void Initialize(XMVECTOR lightColor);
};