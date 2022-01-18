#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct XBoxOneDPad {
  
  ComPtr<ID2D1SolidColorBrush>      dpadBrush;
  ComPtr<ID2D1PathGeometry>         dpadGeometry;
  ComPtr<ID2D1SolidColorBrush>      triangleBrush;
  ComPtr<ID2D1PathGeometry>         triangleGeometryUp;
  ComPtr<ID2D1PathGeometry>         triangleGeometryDown;
  ComPtr<ID2D1PathGeometry>         triangleGeometryLeft;
  ComPtr<ID2D1PathGeometry>         triangleGeometryRight;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID2D1Factory6>	d2d1Factory);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, FLOAT scaleFactor, BOOLEAN upPressed, BOOLEAN downPressed, BOOLEAN leftPressed, BOOLEAN rightPressed);
};

