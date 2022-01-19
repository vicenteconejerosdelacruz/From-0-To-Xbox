#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct Label {
  ComPtr<ID2D1SolidColorBrush>			textBrush;
  ComPtr<IDWriteTextFormat>					textFormat;

  void Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, D2D1::ColorF color = D2D1::ColorF::IndianRed);
  void Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, FLOAT width, FLOAT height, std::wstring label);
};

