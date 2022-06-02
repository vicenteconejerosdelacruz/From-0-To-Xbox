#include "pch.h"
#include "FPSText.h"
#include "../Common/DirectXHelper.h"

void FPSText::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<IDWriteFactory> dWriteFactory) {
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond), &textBrush));
	DX::ThrowIfFailed(dWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32, L"pl-PL", &textFormat));
	DX::ThrowIfFailed(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	DX::ThrowIfFailed(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
}

void FPSText::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, uint32 fps) {
	d2d1DeviceContext->BeginDraw();
	d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F Wnd{};

	int tvOffsetY = 0;
	int tvOffsetX = 0;

	tvOffsetX = 40;
	tvOffsetY = 20;

	Wnd = D2D1::RectF(static_cast<FLOAT>(10 + tvOffsetX), static_cast<FLOAT>(10 + tvOffsetY), static_cast<FLOAT>(250 + tvOffsetX), static_cast<FLOAT>(30 + tvOffsetY));

	std::wstring fpsText = (std::to_wstring(fps) + L" FPS");
	d2d1DeviceContext->DrawText(fpsText.c_str(), static_cast<UINT32>(fpsText.length()), textFormat.Get(), &Wnd, textBrush.Get());

	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}