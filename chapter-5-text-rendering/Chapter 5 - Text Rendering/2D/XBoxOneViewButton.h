#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct XBoxOneViewButton {
  ComPtr<ID2D1SolidColorBrush>			buttonBrush;
  ComPtr<ID2D1SolidColorBrush>			pressedBrush;
  ComPtr<ID2D1PathGeometry>         viewGeometry[2];
  ComPtr<ID2D1SolidColorBrush>			viewBrush;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID2D1Factory6> d2d1Factory);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed);
};

