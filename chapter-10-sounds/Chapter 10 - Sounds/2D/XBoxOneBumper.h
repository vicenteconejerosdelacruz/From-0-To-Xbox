#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct XBoxOneBumper {
  
  ComPtr<ID2D1SolidColorBrush>			buttonBrush;
  ComPtr<ID2D1SolidColorBrush>			pressedBrush;
  ComPtr<ID2D1SolidColorBrush>			textBrush;
  ComPtr<IDWriteTextFormat>					textFormat;
  std::wstring                      buttonLabel;
  ComPtr<ID2D1PathGeometry>         buttonGeometry;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID2D1Factory6>	d2d1Factory, ComPtr<IDWriteFactory> dWriteFactory, std::wstring label, D2D1::ColorF labelColor);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed, BOOL LB);
};

