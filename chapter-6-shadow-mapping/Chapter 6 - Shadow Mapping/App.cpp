#include "pch.h"
#include "App.h"
#include "Renderer/DeviceUtils.h"

#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using Microsoft::WRL::ComPtr;

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true),
	renderer(nullptr)
{
}

void App::Run()
{
	auto Destroy = [this]() -> void {
		cube->Destroy();
		pyramid->Destroy();
		floor->Destroy();
		renderer->Destroy();
	};

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			timer.Tick([&]() {});

			//get the gamepad and keyboard states and update their trackers
			//obtener los estados del gamepad y del teclado y actualizar sus trackers
			auto gamePadState = gamePad->GetState(0);
			auto keyboardState = keyboard->GetState();
			auto mouseState = mouse->GetState();
			buttons.Update(gamePadState);
			keys.Update(keyboardState);

			//check if xbox gamepad is connected
			//chequear si esta conectado el gamepad de la xbox
			if (gamePadState.IsConnected()) {
				if (buttons.a == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[0] = !lightsEnabled[0]; }
				if (buttons.b == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[1] = !lightsEnabled[1]; }
				if (buttons.x == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[2] = !lightsEnabled[2]; }
				if (buttons.y == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[3] = !lightsEnabled[3]; }
				if (buttons.view == GamePad::ButtonStateTracker::RELEASED) { useBlinnPhong = !useBlinnPhong; }
				if (buttons.leftTrigger == GamePad::ButtonStateTracker::RELEASED) { shadowMapsEnabled = !shadowMapsEnabled; }
				if (buttons.rightShoulder == GamePad::ButtonStateTracker::RELEASED) { currentPerspective = (currentPerspective + 1) % 4; }
				if (buttons.leftShoulder == GamePad::ButtonStateTracker::RELEASED) { currentPerspective = (currentPerspective != 0)?(currentPerspective - 1):3; }

				//control the camera using the gamepad
				//controlar la camara usando el gamepad
				if (currentPerspective % 4 == 0) {
					XMVECTOR fw = cameraFw();
					if (gamePadState.thumbSticks.leftY > 0) { cameraPos = XMVectorAdd(cameraPos, fw * cameraSpeed); }
					if (gamePadState.thumbSticks.leftY < 0) { cameraPos = XMVectorAdd(cameraPos, fw * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX < 0) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX > 0) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * cameraSpeed); }
				} else if(currentPerspective % 4 == 2) {
					XMVECTOR fw = spotLight->directionAndAngle(); XMVectorSetW(fw, 0.0f);
					if (gamePadState.thumbSticks.leftY > 0) { spotLight->position = XMVectorAdd(spotLight->position, fw * cameraSpeed); }
					if (gamePadState.thumbSticks.leftY < 0) { spotLight->position = XMVectorAdd(spotLight->position, fw * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX < 0) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX > 0) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
				} else if (currentPerspective % 4 == 3) {
					XMVECTOR fw = cameraFw();
					if (gamePadState.thumbSticks.leftY > 0) { pointLight->position = XMVectorAdd(pointLight->position, fw * cameraSpeed); }
					if (gamePadState.thumbSticks.leftY < 0) { pointLight->position = XMVectorAdd(pointLight->position, fw * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX < 0) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
					if (gamePadState.thumbSticks.leftX > 0) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
				}
			} else {
				//otherwise use the keyboard
				//en caso contrario usamos el teclado
				if (keys.pressed.A) { lightsEnabled[0] = !lightsEnabled[0]; }
				if (keys.pressed.D) { lightsEnabled[1] = !lightsEnabled[1]; }
				if (keys.pressed.S) { lightsEnabled[2] = !lightsEnabled[2]; }
				if (keys.pressed.P) { lightsEnabled[3] = !lightsEnabled[3]; }
				if (keys.pressed.L) { useBlinnPhong = !useBlinnPhong; }
				if (keys.pressed.M) { shadowMapsEnabled = !shadowMapsEnabled; }
				if (keys.pressed.PageDown) { currentPerspective = (currentPerspective + 1) % 4; }
				if (keys.pressed.PageUp) { currentPerspective = (currentPerspective != 0) ? (currentPerspective - 1) : 3; }

				//control the camera using the keyboard
				//controlar la camara usando el teclado
				if (currentPerspective % 4 == 0) {
					XMVECTOR fw = cameraFw();
					if (keyboardState.Up) { cameraPos = XMVectorAdd(cameraPos, fw * cameraSpeed); }
					if (keyboardState.Down) { cameraPos = XMVectorAdd(cameraPos, fw * -cameraSpeed); }
					if (keyboardState.Left) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw,up) * -cameraSpeed); }
					if (keyboardState.Right) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * cameraSpeed); }
				} else if (currentPerspective % 4 == 2) {
					XMVECTOR fw = spotLight->directionAndAngle(); XMVectorSetW(fw, 0.0f);
					if (keyboardState.Up) { spotLight->position = XMVectorAdd(spotLight->position, fw * cameraSpeed); }
					if (keyboardState.Down) { spotLight->position = XMVectorAdd(spotLight->position, fw * -cameraSpeed); }
					if (keyboardState.Left) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
					if (keyboardState.Right) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
				} else if (currentPerspective % 4 == 3) {
					XMVECTOR fw = cameraFw();
					if (keyboardState.Up) { pointLight->position = XMVectorAdd(pointLight->position, fw * cameraSpeed); }
					if (keyboardState.Down) { pointLight->position = XMVectorAdd(pointLight->position, fw * -cameraSpeed); }
					if (keyboardState.Left) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
					if (keyboardState.Right) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
				}
			}

			//toggle fullscreen with the space bar
			//cambia a modo pantalla completa con la barra espaciadora
			if (keys.pressed.Space) {
				auto applicationView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
				if (applicationView->IsFullScreenMode) {
					applicationView->ExitFullScreenMode();
				} else {
					applicationView->TryEnterFullScreenMode();
				}
			}

			//exit the application with escape
			//salir de la aplicacion presionando escape
			if (keys.pressed.Escape) {
				m_windowClosed = true;
				break;
			}

			//track the current mouse position
			//hacer seguimiento de la posicion actual del mouse
			XMFLOAT2 currentMousePos = { static_cast<FLOAT>(mouseState.x) , static_cast<FLOAT>(mouseState.y)};

			//only rotate the camera if the left button is pressed and there is no gamepad
			//solo rota la camara si el boton izquierdo esta presionado y no hay un gamepad
			if (!gamePadState.IsConnected() && mouseState.leftButton) {
				XMFLOAT2 diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
				if (currentPerspective % 4 == 0 || currentPerspective % 4 == 3) {
					cameraRotations.x -= diff.x * 0.001f;
					cameraRotations.y -= diff.y * 0.001f;
				} else if (currentPerspective % 4 == 1) {
					directionalLight->rotation.x -= diff.x * 0.001f;
					directionalLight->rotation.y -= diff.y * 0.001f;
				} else if (currentPerspective % 4 == 2) {
					spotLight->rotation.x -= diff.x * 0.001f;
					spotLight->rotation.y -= diff.y * 0.001f;
				}
			} else if (gamePadState.IsConnected()) {
				if (currentPerspective % 4 == 0 || currentPerspective % 4 == 3) {
					cameraRotations.x -= gamePadState.thumbSticks.rightX * 0.05f;
					cameraRotations.y += gamePadState.thumbSticks.rightY * 0.05f;
				} else if (currentPerspective % 4 == 1) {
					directionalLight->rotation.x -= gamePadState.thumbSticks.rightX * 0.05f;
					directionalLight->rotation.y += gamePadState.thumbSticks.rightY * 0.05f;
				} else if (currentPerspective % 4 == 2) {
					spotLight->rotation.x -= gamePadState.thumbSticks.rightX * 0.05f;
					spotLight->rotation.y += gamePadState.thumbSticks.rightY * 0.05f;
				}
			}

			//store the current mouse position for the next frame
			//guarda la posicion del mouse para el siguiente frame
			lastMousePos = currentMousePos;

			//update cube && pyramid
			cube->Step();
			pyramid->Step();

			renderer->ResetCommands();

			PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render");
			{
				XMMATRIX directionalLightShadowMapViewProjection;
				XMMATRIX spotLightShadowMapViewProjection;
				XMMATRIX pointLightShadowMapViewProjection[6];
				XMVECTOR shadowMapCameraPosition;

				PIXBeginEvent(renderer->commandList.Get(), 0, L"Directional Light ShadowMap");
				{
					renderer->SetShadowMapTarget(
						directionalLight->shadowMap,
						directionalLight->shadowMapDSVDescriptorHeap,
						directionalLight->shadowMapScissorRect,
						directionalLight->shadowMapViewport
					);

					XMVECTOR directionalShadowMapDirection = directionalLight->direction();
					shadowMapCameraPosition = cube->position + XMVectorScale(XMVector3Normalize(directionalShadowMapDirection), -30.0f);
					XMMATRIX shadowMapView = XMMatrixLookAtRH(shadowMapCameraPosition, cube->position, (abs(directionalShadowMapDirection.m128_f32[2])<0.9999f)?up:right);
					directionalLightShadowMapViewProjection = XMMatrixMultiply(shadowMapView, directionalLight->shadowMapPerspectiveMatrix);

					cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, cube->directionalLightShadowMapCbvData);
					cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->directionalLightShadowMapCbvData);

					pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, pyramid->directionalLightShadowMapCbvData);
					pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->directionalLightShadowMapCbvData);
				}
				PIXEndEvent(renderer->commandList.Get());

				PIXBeginEvent(renderer->commandList.Get(), 0, L"Spot Light ShadowMap");
				{
					renderer->SetShadowMapTarget(
						spotLight->shadowMap,
						spotLight->shadowMapDSVDescriptorHeap,
						spotLight->shadowMapScissorRect,
						spotLight->shadowMapViewport
					);
					
					XMMATRIX view = XMMatrixLookToRH(spotLight->position, spotLight->directionAndAngle(), up);
					spotLightShadowMapViewProjection = XMMatrixMultiply(view, spotLight->shadowMapPerspectiveMatrix);

					cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, cube->spotLightShadowMapCbvData);
					cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->spotLightShadowMapCbvData);

					pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, pyramid->spotLightShadowMapCbvData);
					pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->spotLightShadowMapCbvData);
				}
				PIXEndEvent(renderer->commandList.Get());

				PIXBeginEvent(renderer->commandList.Get(), 0, L"Point Light ShadowMap");
				{
					renderer->SetShadowMapTarget(
						pointLight->shadowMap,
						pointLight->shadowMapDSVDescriptorHeap,
						pointLight->shadowMapClearScissorRect,
						pointLight->shadowMapClearViewport
					);

					for (UINT i = 0; i < 6; i++) {
						renderer->commandList->RSSetViewports(1, &pointLight->shadowMapViewport[i]);
						renderer->commandList->RSSetScissorRects(1, &pointLight->shadowMapScissorRect[i]);

						XMMATRIX view = XMMatrixLookToRH(pointLight->position, pointLight->direction[i], pointLight->up[i]);
						pointLightShadowMapViewProjection[i] = XMMatrixMultiply(view, pointLight->shadowMapPerspectiveMatrix);

						cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], cube->pointLightShadowMapCbvData[i]);
						cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->pointLightShadowMapCbvData[i]);

						pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], pyramid->pointLightShadowMapCbvData[i]);
						pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->pointLightShadowMapCbvData[i]);
					}
				}
				PIXEndEvent(renderer->commandList.Get());

				PIXBeginEvent(renderer->commandList.Get(), 0, L"Render Scene");
				{
					renderer->SetRenderTargets();

					XMMATRIX view, viewProjection;
					XMVECTOR cameraPosition;

					if (currentPerspective % 4 == 0) {
						view = XMMatrixLookToRH(cameraPos, cameraFw(), up);
						viewProjection = XMMatrixMultiply(view, renderer->perspectiveMatrix);
						cameraPosition = cameraPos;
					} else if (currentPerspective % 4 == 1) {
						viewProjection = directionalLightShadowMapViewProjection;
						cameraPosition = shadowMapCameraPosition;
					} else if (currentPerspective % 4 == 2) {
						viewProjection = spotLightShadowMapViewProjection;
						cameraPosition = spotLight->position;
					} else if (currentPerspective % 4 == 3) {
						view = XMMatrixLookToRH(pointLight->position, cameraFw(), up);
						viewProjection = XMMatrixMultiply(view, pointLight->shadowMapPerspectiveMatrix);
						cameraPosition = pointLight->position;
					}
					//update cube constants and draw
					//actualizar las constantes del cubo y dibujarlo
					cube->UpdateConstantsBuffer(
						renderer->backBufferIndex,
						useBlinnPhong,
						viewProjection,
						cameraPosition,
						lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
						directionalLight->direction(),
						lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
						spotLight->position,
						lightsEnabled[2] ? spotLight->color : XMVectorZero(),
						spotLight->directionAndAngle(),
						spotLight->attenuation,
						lightsEnabled[3] ? pointLight->color : XMVectorZero(),
						pointLight->position,
						pointLight->attenuation,
						shadowMapsEnabled,
						directionalLightShadowMapViewProjection,
						directionalLight->shadowMapTexelInvSize,
						spotLightShadowMapViewProjection,
						spotLight->shadowMapTexelInvSize,
						pointLightShadowMapViewProjection
					);
					cube->Render(renderer->commandList, renderer->backBufferIndex);

					//update pyramid constants and draw
					//actualizar las constantes de la piramide y dibujarla
					pyramid->UpdateConstantsBuffer(
						renderer->backBufferIndex,
						useBlinnPhong,
						viewProjection,
						cameraPosition,
						lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
						directionalLight->direction(),
						lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
						spotLight->position,
						lightsEnabled[2] ? spotLight->color : XMVectorZero(),
						spotLight->directionAndAngle(),
						spotLight->attenuation,
						lightsEnabled[3] ? pointLight->color : XMVectorZero(),
						pointLight->position,
						pointLight->attenuation,
						shadowMapsEnabled,
						directionalLightShadowMapViewProjection,
						directionalLight->shadowMapTexelInvSize,
						spotLightShadowMapViewProjection,
						spotLight->shadowMapTexelInvSize,
						pointLightShadowMapViewProjection
					);
					pyramid->Render(renderer->commandList, renderer->backBufferIndex);

					//update floor constants and draw
					//actualizar las constantes del suelo y dibujarlo
					floor->UpdateConstantsBuffer(
						renderer->backBufferIndex,
						useBlinnPhong,
						viewProjection,
						cameraPosition,
						lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
						directionalLight->direction(),
						lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
						spotLight->position,
						lightsEnabled[2] ? spotLight->color : XMVectorZero(),
						spotLight->directionAndAngle(),
						spotLight->attenuation,
						lightsEnabled[3] ? pointLight->color : XMVectorZero(),
						pointLight->position,
						pointLight->attenuation,
						shadowMapsEnabled,
						directionalLightShadowMapViewProjection,
						directionalLight->shadowMapTexelInvSize,
						spotLightShadowMapViewProjection,
						spotLight->shadowMapTexelInvSize,
						pointLightShadowMapViewProjection
					);
					floor->Render(renderer->commandList, renderer->backBufferIndex);
				}
				PIXEndEvent(renderer->commandList.Get());

				//before switching to 2D mode the commandQueue must be executed
				//antes de pasar a modo 2D el commandQueue debe ser ejecutado
				renderer->ExecuteCommands();

				//PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render UI");
				{
					//switch to 2d mode
					//cambiar a modo 2D
					renderer->Set2DRenderTarget();

					//draw the framerate
					//dibujar el framerate
					fpsText->Render(renderer->d2d1DeviceContext, timer.GetFramesPerSecond());

					const LPWSTR perspectiveSelection[] = {
						L"Scene Camera",
						L"Directional Light",
						L"Spot Light",
						L"Point Light"
					};
					perspectiveSelectionLabel->Render(renderer->d2d1DeviceContext, 200, 10, 500, 300, perspectiveSelection[currentPerspective%_countof(perspectiveSelection)]);

					if (gamePadState.IsConnected()) {
						//draw the XBoxOne buttons
						//dibujar los botones de la XBoxOne
						const D2D1_POINT_2F gamePadOffsets[] = {
							D2D1::Point2F(100.0f, 40.0f),
							D2D1::Point2F( 60.0f, 80.0f),
							D2D1::Point2F(140.0f, 80.0f),
							D2D1::Point2F(100.0f,120.0f),
							D2D1::Point2F(200.0f,120.0f),
							D2D1::Point2F(200.0f,45.0f),
							D2D1::Point2F(400.0f,45.0f),
							D2D1::Point2F(40.0f,230.0f)
						};
						xboxOneButtonA->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[0].x, renderer->window->Bounds.Height - gamePadOffsets[0].y, buttons.a == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonB->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[1].x, renderer->window->Bounds.Height - gamePadOffsets[1].y, buttons.b == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonX->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[2].x, renderer->window->Bounds.Height - gamePadOffsets[2].y, buttons.x == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonY->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[3].x, renderer->window->Bounds.Height - gamePadOffsets[3].y, buttons.y == GamePad::ButtonStateTracker::HELD);
						xboxOneViewButton->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[4].x, renderer->window->Bounds.Height - gamePadOffsets[4].y, buttons.view == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonLB->Render(renderer->d2d1DeviceContext, gamePadOffsets[5].x, gamePadOffsets[5].y, buttons.leftShoulder == GamePad::ButtonStateTracker::HELD, TRUE);
						xboxOneButtonRB->Render(renderer->d2d1DeviceContext, gamePadOffsets[6].x, gamePadOffsets[6].y, buttons.rightShoulder == GamePad::ButtonStateTracker::HELD, FALSE);
						xboxOneButtonLT->Render(renderer->d2d1DeviceContext, gamePadOffsets[7].x, renderer->window->Bounds.Height - gamePadOffsets[7].y, buttons.leftTrigger == GamePad::ButtonStateTracker::HELD, FALSE);

						ambientLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 130, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
						directionalLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 80, renderer->window->Bounds.Height - 60, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
						spotLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 170, renderer->window->Bounds.Height - 60, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
						pointLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 125, renderer->window->Bounds.Height - 155, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
						lightModelLabelCentered->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 250, renderer->window->Bounds.Height - 110, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
						shadowMapsLabel->Render(renderer->d2d1DeviceContext, 10, renderer->window->Bounds.Height - 190, 100.0f, 20.0f, shadowMapsEnabled ? L"Shadow Maps:ON" : L"Shadow Maps:OFF");
						perspectiveSelectionLabelPrevious->Render(renderer->d2d1DeviceContext, 200, 70, 100.0f, 20.0f, ((currentPerspective % 4) == 0)? perspectiveSelection[3]:perspectiveSelection[((currentPerspective - 1) % 4)]);
						perspectiveSelectionLabelNext->Render(renderer->d2d1DeviceContext, 360, 70, 100.0f, 20.0f, ((currentPerspective % 4) == 3)? perspectiveSelection[0]:perspectiveSelection[((currentPerspective + 1) % 4)]);
					} else {
						//draw the keyboard buttons (S<->D swapped because it looks better)
						//dibujar los botones del teclado (S<->D cambiados por que se ve mejor)
						keyboardButtonA->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (1.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.A);
						keyboardButtonD->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (3.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.D);
						keyboardButtonS->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (2.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.S);
						keyboardButtonP->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (4.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.P);
						keyboardButtonL->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (5.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.L);
						keyboardButtonM->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (6.0f / 7.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.M);
						keyboardButtonPU->Render(renderer->d2d1DeviceContext, 200, 65, keyboardState.PageUp);
						keyboardButtonPD->Render(renderer->d2d1DeviceContext, 400, 65, keyboardState.PageDown);

						ambientLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (1.0f / 7.0f) - 30.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
						directionalLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (3.0f / 7.0f) - 30.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
						spotLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (2.0f / 7.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
						pointLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (4.0f / 7.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
						lightModelLabelLeft->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width* (5.0f / 7.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
						shadowMapsLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width* (6.0f / 7.0f) - 40.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, shadowMapsEnabled ? L"Shadow Maps:ON" : L"Shadow Maps:OFF");
						perspectiveSelectionLabelPrevious->Render(renderer->d2d1DeviceContext, 180, 85, 100.0f, 20.0f, ((currentPerspective % 4) == 0) ? perspectiveSelection[3]: perspectiveSelection[((currentPerspective - 1) % 4)]);
						perspectiveSelectionLabelNext->Render(renderer->d2d1DeviceContext, 380, 85, 100.0f, 20.0f, ((currentPerspective % 4) == 3) ? perspectiveSelection[0]: perspectiveSelection[((currentPerspective + 1) % 4)]);
					}
				}
				//PIXEndEvent(renderer->commandQueue.Get());

				renderer->Present();
			}
			PIXEndEvent(renderer->commandQueue.Get());
		} else {
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
	Flush(renderer->commandQueue, renderer->fence, renderer->fenceValue, renderer->fenceEvent);
	Destroy();
}

// El primer método al que se llama cuando se crea IFrameworkView.
void App::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);
}

void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

	//intialize xbox gamepad
	//inicializar el control de xbox
	gamePad = std::make_unique<GamePad>();
	buttons.Reset();

	//initialize keyboard
	//inicializar el teclado
	keyboard = std::make_unique<Keyboard>();
	keyboard->SetWindow(window);

	//initialize the mouse
	//inicializar el mouse
	mouse = std::make_unique<Mouse>();
	mouse->SetWindow(window);

	//initialize direct3d and the device resources(swapchain,backbuffers,depthstencil)
	//inicializar direct3d y los recursos del device(swapchain,backbuffers,depthstencil)
	renderer = std::make_shared<Renderer>();
	renderer->Initialize(CoreWindow::GetForCurrentThread());

	//reset the GPU command list to start recording cube initialization commands
	//reinicia la lista de comandos de la GPU para poder cargar el cubo
	renderer->ResetCommands();

	//intialize the lights on the scene
	//inicializar las luces de la escena
	directionalLight = std::make_shared<DirectionalLight>();
	directionalLight->Initialize(
		{ 0.3f, 0.3f, 0.4f, 0.0f }, //color
		{ -DirectX::XM_PIDIV4 , -DirectX::XM_PIDIV4 },
		renderer->d3dDevice,
		1024U,
		1024U,
		16.0f,16.0f,1000.0f
	);

	spotLight = std::make_shared<SpotLight>();
	spotLight->Initialize(
		{ 0.3f, 0.3f, 0.4f, 0.0f }, //color
		{ -4.0f, 2.75f, -3.0f, 0.0f }, //position
		{ 1.0f*DirectX::XM_PIDIV4 , -0.75f*DirectX::XM_PIDIV4 }, //rotation
		DirectX::XM_PIDIV4 * 0.5f, //fov
		{ 0.0f, 0.1f, 0.02f, 0.0f }, //attenuation
		renderer->d3dDevice,
		1024U,
		1024U,
		10.0f
	);

	pointLight = std::make_shared<PointLight>();
	pointLight->Initialize(
		{ 0.1f, 0.7f, 0.1f, 0.0f }, //color
		{ -5.0f, 3.0f, -1.5f, 0.0f }, //position
		{ 0.0f, 0.1f, 0.02f, 0.0f }, //attenuation
		renderer->d3dDevice,
		1024U,
		1024U,
		20.0f
	);

	ambientLight = std::make_shared<AmbientLight>();
	ambientLight->Initialize(
		{ 0.1f, 0.1f, 0.1f, 0.0f }
	);

	//intialize 3d Objects on the scene
	//incializar los objetos 3D de la escena
	cube = std::make_shared<Cube>();
	cube->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);

	pyramid = std::make_shared<Pyramid>();
	pyramid->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);
	pyramid->position = XMVectorSetX(pyramid->position, 2.5f);

	floor = std::make_shared<Floor>();
	floor->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);

	//initialize 2D objects
	//inicializar objetos 2D
	fpsText = std::make_shared<FPSText>();
	fpsText->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory);
	
	//initialize the labels
	//inicializar los labels
	ambientLightLabel = std::make_shared<Label>(); ambientLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	directionalLightLabel = std::make_shared<Label>(); directionalLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	spotLightLabel = std::make_shared<Label>(); spotLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	pointLightLabel = std::make_shared<Label>(); pointLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	lightModelLabelCentered = std::make_shared<Label>(); lightModelLabelCentered->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	lightModelLabelLeft = std::make_shared<Label>(); lightModelLabelLeft->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	shadowMapsLabel = std::make_shared<Label>(); shadowMapsLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	perspectiveSelectionLabel = std::make_shared<Label>(); perspectiveSelectionLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 32, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR,D2D1::ColorF::White);
	perspectiveSelectionLabelPrevious = std::make_shared<Label>(); perspectiveSelectionLabelPrevious->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
	perspectiveSelectionLabelNext = std::make_shared<Label>(); perspectiveSelectionLabelNext->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);

	//initialize the keyboard buttons
	//inicializar los botones del teclado
	keyboardButtonA = std::make_shared<KeyboardButton>(); keyboardButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0xffffff));
	keyboardButtonS = std::make_shared<KeyboardButton>(); keyboardButtonS->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"S", D2D1::ColorF(0xffffff));
	keyboardButtonD = std::make_shared<KeyboardButton>(); keyboardButtonD->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"D", D2D1::ColorF(0xffffff));
	keyboardButtonP = std::make_shared<KeyboardButton>(); keyboardButtonP->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"P", D2D1::ColorF(0xffffff));
	keyboardButtonL = std::make_shared<KeyboardButton>(); keyboardButtonL->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"L", D2D1::ColorF(0xffffff));
	keyboardButtonM = std::make_shared<KeyboardButton>(); keyboardButtonM->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"M", D2D1::ColorF(0xffffff));
	keyboardButtonPU = std::make_shared<KeyboardButton>(); keyboardButtonPU->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Page Up", D2D1::ColorF(0xffffff), 10.0f);
	keyboardButtonPD = std::make_shared<KeyboardButton>(); keyboardButtonPD->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Page Down", D2D1::ColorF(0xffffff), 10.0f);

	//initialize the xbox buttons
	//inicializar los botones de xbox
	xboxOneButtonA = std::make_shared<XBoxOneButton>(); xboxOneButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0x3cdb4e));
	xboxOneButtonB = std::make_shared<XBoxOneButton>(); xboxOneButtonB->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"B", D2D1::ColorF(0xd04242));
	xboxOneButtonX = std::make_shared<XBoxOneButton>(); xboxOneButtonX->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"X", D2D1::ColorF(0x40ccd0));
	xboxOneButtonY = std::make_shared<XBoxOneButton>(); xboxOneButtonY->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Y", D2D1::ColorF(0xecdb33));
	xboxOneViewButton = std::make_shared<XBoxOneViewButton>(); xboxOneViewButton->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory);
	xboxOneButtonLT = std::make_shared<XBoxOneTrigger>(); xboxOneButtonLT->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"LT", D2D1::ColorF::White);
	xboxOneButtonLB = std::make_shared<XBoxOneBumper>(); xboxOneButtonLB->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"LB", D2D1::ColorF::White);
	xboxOneButtonRB = std::make_shared<XBoxOneBumper>(); xboxOneButtonRB->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"RB", D2D1::ColorF::White);

	//execute the commands on the GPU and wait for it's completion
	//ejecuta los comandos de la GPU y esperar a que termine
	renderer->CloseCommandsAndFlush();

	cube->DestroyUploadResources();
	pyramid->DestroyUploadResources();
	floor->DestroyUploadResources();
}

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		deferral->Complete();
	});
}

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	if (renderer != nullptr) {
		renderer->Resize(static_cast<UINT>(sender->Bounds.Width), static_cast<UINT>(sender->Bounds.Height));
	}
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}
