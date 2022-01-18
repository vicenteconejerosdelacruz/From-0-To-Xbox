#include "pch.h"
#include "XBoxOneDPad.h"
#include "../Common/DirectXHelper.h"

void XBoxOneDPad::Initialize(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID2D1Factory6>	d2d1Factory) {
	
	const D2D1_POINT_2F points[] = {
		D2D1::Point2F(  0, 63),
		D2D1::Point2F( 64, 63),
		D2D1::Point2F( 64,  0),
		D2D1::Point2F(125,  0),
		D2D1::Point2F(125, 63),
		D2D1::Point2F(188, 63),
		D2D1::Point2F(188,124),
		D2D1::Point2F(125,124),
		D2D1::Point2F(125,188),
		D2D1::Point2F( 64,188),
		D2D1::Point2F( 64,124),
		D2D1::Point2F(  0,124),
	};

	const D2D1_POINT_2F trianglePointsUp[] = {
		D2D1::Point2F( 74, 42),
		D2D1::Point2F( 94, 15),
		D2D1::Point2F(115, 42),
	};

	const D2D1_POINT_2F trianglePointsDown[] = {
		D2D1::Point2F( 74,145),
		D2D1::Point2F( 94,172),
		D2D1::Point2F(115,145),
	};

	const D2D1_POINT_2F trianglePointsLeft[] = {
		D2D1::Point2F( 42, 74),
		D2D1::Point2F( 15, 94),
		D2D1::Point2F( 42,115),
	};

	const D2D1_POINT_2F trianglePointsRight[] = {
		D2D1::Point2F(145, 74),
		D2D1::Point2F(172, 94),
		D2D1::Point2F(145,115),
	};
	
	ID2D1GeometrySink* sink = NULL;

	//create the dpad geometry
	//creamos la geometria del dpad
	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&dpadGeometry));
	DX::ThrowIfFailed(dpadGeometry->Open(&sink));

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(points + 1, _countof(points) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	//create a geometry for the top triangle
	//creamos la geometria del triangulo de arriba
	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&triangleGeometryUp));
	DX::ThrowIfFailed(triangleGeometryUp->Open(&sink));

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(trianglePointsUp[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(trianglePointsUp + 1, _countof(trianglePointsUp) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	//create a geometry for the bottom triangle
	//creamos la geometria del triangulo de abajo
	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&triangleGeometryDown));
	DX::ThrowIfFailed(triangleGeometryDown->Open(&sink));

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(trianglePointsDown[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(trianglePointsDown + 1, _countof(trianglePointsDown) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	//create a geometry for the left triangle
	//creamos la geometria del triangulo izquierdo
	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&triangleGeometryLeft));
	DX::ThrowIfFailed(triangleGeometryLeft->Open(&sink));

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(trianglePointsLeft[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(trianglePointsLeft + 1, _countof(trianglePointsLeft) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	//create a geometry for the right triangle
	//creamos la geometria del triangulo derecho
	DX::ThrowIfFailed(d2d1Factory->CreatePathGeometry(&triangleGeometryRight));
	DX::ThrowIfFailed(triangleGeometryRight->Open(&sink));

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	sink->BeginFigure(trianglePointsRight[0], D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(trianglePointsRight + 1, _countof(trianglePointsRight) - 1);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	DX::ThrowIfFailed(sink->Close());

	//create the dpad and triangle brushes
	//creamos los brushs del dpad y el triangulo
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &dpadBrush));
	DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0xecdb33)), &triangleBrush));
}

void XBoxOneDPad::Render(ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, FLOAT x, FLOAT y, FLOAT scaleFactor, BOOLEAN upPressed, BOOLEAN downPressed, BOOLEAN leftPressed, BOOLEAN rightPressed) {
	d2d1DeviceContext->BeginDraw();
	
	D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(scaleFactor, scaleFactor, { 0.0f,0.0f });
	D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(x, y);
	D2D1::Matrix3x2F transformation = scale * translation;
	d2d1DeviceContext->SetTransform(transformation);

	d2d1DeviceContext->FillGeometry(dpadGeometry.Get(), dpadBrush.Get());

	if (upPressed) {
		d2d1DeviceContext->FillGeometry(triangleGeometryUp.Get(), triangleBrush.Get());
	}

	if (downPressed) {
		d2d1DeviceContext->FillGeometry(triangleGeometryDown.Get(), triangleBrush.Get());
	}

	if (leftPressed) {
		d2d1DeviceContext->FillGeometry(triangleGeometryLeft.Get(), triangleBrush.Get());
	}

	if (rightPressed) {
		d2d1DeviceContext->FillGeometry(triangleGeometryRight.Get(), triangleBrush.Get());
	}
	
	DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
}