#include "pch.h"
#include "Label.h"
#include "../Common/DirectXHelper.h"

void Label::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, D2D1::ColorF color) {
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(color, &textBrush));
	DX::ThrowIfFailed(dWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"pl-PL", &textFormat));
	DX::ThrowIfFailed(textFormat->SetTextAlignment(textAlignment));
	DX::ThrowIfFailed(textFormat->SetParagraphAlignment(paragraphAlignment));
}

void Label::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, FLOAT width, FLOAT height, std::wstring label) {
	d2d1DeviceContext->BeginDraw();
	d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F Wnd{};

	Wnd = D2D1::RectF(x, y, x + width, y + height);

	d2d1DeviceContext->DrawText(label.c_str(), static_cast<UINT32>(label.length()), textFormat.Get(), &Wnd, textBrush.Get());

	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}