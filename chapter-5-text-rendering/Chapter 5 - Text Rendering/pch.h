#pragma once

#define USE_PIX
#define USE_PIX_SUPPORTED_ARCHITECTURE

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <dwrite_2.h>
#include <d2d1_3.h>
#include "Common\d3dx12.h"
#include <pix3.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include <GamePad.h>
#include <Keyboard.h>
#include <memory>
#include <vector>
#include <agile.h>
#include <concrt.h>
#include <string>

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif
