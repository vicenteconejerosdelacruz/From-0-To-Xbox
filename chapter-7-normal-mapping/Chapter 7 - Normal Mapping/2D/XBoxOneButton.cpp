#include "pch.h"
#include "XBoxOneButton.h"
#include "../Common/DirectXHelper.h"

void XBoxOneButton::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory, std::wstring label, D2D1::ColorF labelColor) {
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(labelColor), &textBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &buttonBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0xecdb33)), &pressedBrush));
	DX::ThrowIfFailed(dWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32, L"pl-PL", &textFormat));
	DX::ThrowIfFailed(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	DX::ThrowIfFailed(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	buttonLabel = label;
}

void XBoxOneButton::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed) {
	d2d1DeviceContext->BeginDraw();
	d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F Wnd{};

	Wnd = D2D1::RectF(x - 10.0f, y - 10.0f, x + 10.0f, y + 10.0f);
	D2D1_ELLIPSE circle = { D2D1::Point2F(x,y), 20.0f, 20.0f };

	d2d1DeviceContext->FillEllipse(circle, buttonBrush.Get());
	if (pressed) {
		d2d1DeviceContext->DrawEllipse(circle, pressedBrush.Get(),3.0f);
	}
	d2d1DeviceContext->DrawText(buttonLabel.c_str(), static_cast<UINT32>(buttonLabel.length()), textFormat.Get(), &Wnd, textBrush.Get());

	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}