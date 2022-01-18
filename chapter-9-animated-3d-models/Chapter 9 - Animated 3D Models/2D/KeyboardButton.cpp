#include "pch.h"
#include "KeyboardButton.h"
#include "../Common/DirectXHelper.h"

void KeyboardButton::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory, std::wstring label, D2D1::ColorF labelColor, FLOAT fontSize) {
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(labelColor), &textBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &buttonBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0xecdb33)), &pressedBrush));
	DX::ThrowIfFailed(dWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"pl-PL", &textFormat));
	DX::ThrowIfFailed(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	DX::ThrowIfFailed(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	buttonLabel = label;
}

void KeyboardButton::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed) {
	d2d1DeviceContext->BeginDraw();
	d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F rect = D2D1::RectF(x - 20.0f, y - 20.0f, x + 20.0f, y + 20.0f);
	D2D1_ROUNDED_RECT roundedRect = { rect, 5.0f, 5.0f };

	d2d1DeviceContext->FillRoundedRectangle(roundedRect, buttonBrush.Get());
	if (pressed) {
		d2d1DeviceContext->DrawRoundedRectangle(roundedRect, pressedBrush.Get(),3.0f);
	}
	d2d1DeviceContext->DrawText(buttonLabel.c_str(), static_cast<UINT32>(buttonLabel.length()), textFormat.Get(), &rect, textBrush.Get());

	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}