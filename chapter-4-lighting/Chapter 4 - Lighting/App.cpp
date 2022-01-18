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
					ambientLightColor,
					directionalLightDirection,
					directionalLightColor,
					spotLightPosition,
					spotLightColor,
					spotLightDirectionAndAngle,
					spotLightAttenuation,
					pointLightColor,
					pointLightPosition,
					pointLightAttenuation
				);
				cube->Render(renderer->commandList, renderer->backBufferIndex);

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

	//initialize direct3d and the device resources(swapchain,backbuffers,depthstencil)
	//inicializar direct3d y los recursos del device(swapchain,backbuffers,depthstencil)
	renderer = std::make_shared<Renderer>();
	renderer->Initialize(CoreWindow::GetForCurrentThread());

	//reset the GPU command list to start recording cube initialization commands
	//reinicia la lista de comandos de la GPU para poder cargar el cubo
	renderer->ResetCommands();

	cube = std::make_shared<Cube>();
	cube->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList);
	
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
