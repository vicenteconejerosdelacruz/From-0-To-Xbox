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
			} else {
				//otherwise use the keyboard
				//en caso contrario usamos el teclado
				if (keys.pressed.A) { lightsEnabled[0] = !lightsEnabled[0]; }
				if (keys.pressed.D) { lightsEnabled[1] = !lightsEnabled[1]; }
				if (keys.pressed.S) { lightsEnabled[2] = !lightsEnabled[2]; }
				if (keys.pressed.P) { lightsEnabled[3] = !lightsEnabled[3]; }
				if (keys.pressed.L) { useBlinnPhong = !useBlinnPhong; }
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
			renderer->ResetCommands();

			PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render");
			{
				renderer->SetRenderTargets();

				XMMATRIX view = XMMatrixLookToRH(cameraPos, cameraFw, up);
				XMMATRIX viewProjection = XMMatrixMultiply(view, renderer->perspectiveMatrix);

				cube->UpdateConstantsBuffer(
					renderer->backBufferIndex,
					useBlinnPhong,
					viewProjection,
					cameraPos,
					lightsEnabled[0] ? ambientLightColor : XMVectorZero(),
					directionalLightDirection,
					lightsEnabled[1] ? directionalLightColor : XMVectorZero(),
					spotLightPosition,
					lightsEnabled[2] ? spotLightColor : XMVectorZero(),
					spotLightDirectionAndAngle,
					spotLightAttenuation,
					lightsEnabled[3] ? pointLightColor : XMVectorZero(),
					pointLightPosition,
					pointLightAttenuation
				);
				cube->Render(renderer->commandList, renderer->backBufferIndex);

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

					if (gamePadState.IsConnected()) {
						//draw the XBoxOne buttons
						//dibujar los botones de la XBoxOne
						const D2D1_POINT_2F gamePadOffsets[] = {
							D2D1::Point2F(100.0f, 40.0f),
							D2D1::Point2F( 60.0f, 80.0f),
							D2D1::Point2F(140.0f, 80.0f),
							D2D1::Point2F(100.0f,120.0f),
							D2D1::Point2F(200.0f,120.0f)
						};
						xboxOneButtonA->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[0].x, renderer->window->Bounds.Height - gamePadOffsets[0].y, buttons.a == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonB->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[1].x, renderer->window->Bounds.Height - gamePadOffsets[1].y, buttons.b == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonX->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[2].x, renderer->window->Bounds.Height - gamePadOffsets[2].y, buttons.x == GamePad::ButtonStateTracker::HELD);
						xboxOneButtonY->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[3].x, renderer->window->Bounds.Height - gamePadOffsets[3].y, buttons.y == GamePad::ButtonStateTracker::HELD);
						xboxOneViewButton->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - gamePadOffsets[4].x, renderer->window->Bounds.Height - gamePadOffsets[4].y, buttons.view == GamePad::ButtonStateTracker::HELD);

						ambientLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 130, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
						directionalLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 80, renderer->window->Bounds.Height - 60, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
						spotLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 170, renderer->window->Bounds.Height - 60, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
						pointLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 125, renderer->window->Bounds.Height - 155, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
						lightModelLabelCentered->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width - 250, renderer->window->Bounds.Height - 110, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
					} else {
						//draw the keyboard buttons (S<->D swapped because it looks better)
						//dibujar los botones del teclado (S<->D cambiados por que se ve mejor)
						keyboardButtonA->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (1.0f / 6.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.A);
						keyboardButtonD->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (3.0f / 6.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.D);
						keyboardButtonS->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (2.0f / 6.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.S);
						keyboardButtonP->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (4.0f / 6.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.P);
						keyboardButtonL->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (5.0f / 6.0f), renderer->window->Bounds.Height - 40.0f, keyboardState.L);

						ambientLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (1.0f / 6.0f) - 30.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
						directionalLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (3.0f / 6.0f) - 30.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
						spotLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (2.0f / 6.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
						pointLightLabel->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width * (4.0f / 6.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
						lightModelLabelLeft->Render(renderer->d2d1DeviceContext, renderer->window->Bounds.Width* (5.0f / 6.0f) - 20.0f, renderer->window->Bounds.Height - 20, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
					}
					//PIXEndEvent(renderer->commandQueue.Get());

					renderer->Present();
				}
				PIXEndEvent(renderer->commandQueue.Get());
			}
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

	//initialize direct3d and the device resources(swapchain,backbuffers,depthstencil)
	//inicializar direct3d y los recursos del device(swapchain,backbuffers,depthstencil)
	renderer = std::make_shared<Renderer>();
	renderer->Initialize(CoreWindow::GetForCurrentThread());

	//reset the GPU command list to start recording cube initialization commands
	//reinicia la lista de comandos de la GPU para poder cargar el cubo
	renderer->ResetCommands();

	cube = std::make_shared<Cube>();
	cube->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList);

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

	//initialize the keyboard buttons
	//inicializar los botones del teclado
	keyboardButtonA = std::make_shared<KeyboardButton>(); keyboardButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0xffffff));
	keyboardButtonS = std::make_shared<KeyboardButton>(); keyboardButtonS->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"S", D2D1::ColorF(0xffffff));
	keyboardButtonD = std::make_shared<KeyboardButton>(); keyboardButtonD->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"D", D2D1::ColorF(0xffffff));
	keyboardButtonP = std::make_shared<KeyboardButton>(); keyboardButtonP->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"P", D2D1::ColorF(0xffffff));
	keyboardButtonL = std::make_shared<KeyboardButton>(); keyboardButtonL->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"L", D2D1::ColorF(0xffffff));

	//initialize the xbox buttons
	//inicializar los botones de xbox
	xboxOneButtonA = std::make_shared<XBoxOneButton>(); xboxOneButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0x3cdb4e));
	xboxOneButtonB = std::make_shared<XBoxOneButton>(); xboxOneButtonB->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"B", D2D1::ColorF(0xd04242));
	xboxOneButtonX = std::make_shared<XBoxOneButton>(); xboxOneButtonX->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"X", D2D1::ColorF(0x40ccd0));
	xboxOneButtonY = std::make_shared<XBoxOneButton>(); xboxOneButtonY->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Y", D2D1::ColorF(0xecdb33));
	xboxOneViewButton = std::make_shared<XBoxOneViewButton>(); xboxOneViewButton->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory);

	//execute the commands on the GPU and wait for it's completion
	//ejecuta los comandos de la GPU y esperar a que termine
	renderer->CloseCommandsAndFlush();

	cube->DestroyUploadResources();
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
