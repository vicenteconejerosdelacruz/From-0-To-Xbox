#include "pch.h"
#include "XBoxOneViewButton.h"
#include "../Common/DirectXHelper.h"

void XBoxOneViewButton::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID2D1Factory6>	d2d1Factory) {

	const D2D1_POINT_2F pointsView1[] = {
		D2D1::Point2F(2,  5),
		D2D1::Point2F(0,  5),
		D2D1::Point2F(0,  0),
		D2D1::Point2F(8,  0),
		D2D1::Point2F(8,  2),
	};

	const D2D1_POINT_2F pointsView2[] = {
		D2D1::Point2F(4,  10),
		D2D1::Point2F(4,  4),
		D2D1::Point2F(12,  4),
		D2D1::Point2F(12,  10)
	};

	ID2D1GeometrySink* sink = NULL;

	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&viewGeometry[0]));
	DX::ThrowIfFailed(viewGeometry[0]->Open(&sink));
	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(pointsView1[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(pointsView1 + 1, _countof(pointsView1) - 1);
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	DX::ThrowIfFailed(sink->Close());

	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&viewGeometry[1]));
	DX::ThrowIfFailed(viewGeometry[1]->Open(&sink));
	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(pointsView2[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(pointsView2 + 1, _countof(pointsView2) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &buttonBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0xecdb33)), &pressedBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &viewBrush));
}

void XBoxOneViewButton::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, BOOLEAN pressed) {
	d2d1DeviceContext->BeginDraw();
	d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	D2D1_RECT_F Wnd{};

	Wnd = D2D1::RectF(x - 5.0f, y - 5.0f, x + 5.0f, y + 5.0f);
	D2D1_ELLIPSE circle = { D2D1::Point2F(x,y), 10.0f, 10.0f };

	d2d1DeviceContext->FillEllipse(circle, buttonBrush.Get());
	if (pressed) {
		d2d1DeviceContext->DrawEllipse(circle, pressedBrush.Get(),3.0f);
	}

	D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(x-5, y-5);
	d2d1DeviceContext->SetTransform(translation);

	d2d1DeviceContext->DrawGeometry(viewGeometry[0].Get(), viewBrush.Get(), 1.0f);
	d2d1DeviceContext->DrawGeometry(viewGeometry[1].Get(), viewBrush.Get(), 1.0f);

	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}