#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct FPSText {
  ComPtr<ID2D1SolidColorBrush>			textBrush;
  ComPtr<IDWriteTextFormat>					textFormat;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, uint32 fps);
};

