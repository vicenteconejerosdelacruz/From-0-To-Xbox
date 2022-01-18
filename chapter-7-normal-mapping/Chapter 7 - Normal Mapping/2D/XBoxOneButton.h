#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct XBoxOneButton {
  ComPtr<ID2D1SolidColorBrush>			buttonBrush;
  ComPtr<ID2D1SolidColorBrush>			pressedBrush;
  ComPtr<ID2D1SolidColorBrush>			textBrush;
  ComPtr<IDWriteTextFormat>					textFormat;
  std::wstring                      buttonLabel;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory, std::wstring label, D2D1::ColorF labelColor);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed);
};

