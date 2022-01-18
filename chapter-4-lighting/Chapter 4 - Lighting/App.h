#pragma once

#include "pch.h"
#include "Renderer/Renderer.h"
#include "Primitives/Cube.h"

ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	App();

	// Métodos IFrameworkView.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint) {}
	virtual void Run();
	virtual void Uninitialize() {}

protected:
	// Controladores de eventos del ciclo de vida de la aplicación.
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);

	// Controladores de eventos de Windows.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

private:
	bool m_windowClosed;
	bool m_windowVisible;
	std::shared_ptr<Renderer> renderer;
	
	//3d objects in scene
	//objetos 3d en la escena
	std::shared_ptr<Cube> cube;

	//camera properties
	//propiedades de la camara
	XMVECTOR	cameraPos = { 0.0f, 0.0f, -5.5f, 0.0f };
	XMVECTOR	cameraFw = { 0.0f, 0.0f, 1.0f, 0.0f };
	XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };

	/*Blinn-Phong:true*/
	BOOL useBlinnPhong = TRUE;

	//directional light properties
	//propiedades de la luz direcional 
	XMVECTOR directionalLightColor = { 0.3f, 0.3f, 0.4f, 0.0f };
	XMVECTOR directionalLightDirection = { 1.0f, -1.0f, 3.0f, 0.0f };

	//spot light properties
	//propiedades de la luz direcional
	XMVECTOR spotLightColor = { 0.3f, 0.3f, 0.4f, 0.0f };
	XMVECTOR spotLightPosition = { -2.5f, -1.25f, -3.0f, 0.0f };
	XMVECTOR spotLightDirectionAndAngle = { 0.9f, 0.0f, 1.0f, DirectX::XMScalarCos(DirectX::XM_PIDIV4*0.25f) };
	XMVECTOR spotLightAttenuation = { 0.0f, 0.1f, 0.02f, 0.0f };

	//point light properties
	//propiedades de la luz omni-direcional
	XMVECTOR pointLightColor = { 0.1f, 0.7f, 0.1f, 0.0f };
	XMVECTOR pointLightPosition = { 2.5f, 0.0f, -2.5f, 0.0f };
	XMVECTOR pointLightAttenuation = { 0.0f, 0.3f, 0.08f, 0.0f };

	//ambient light properties
	//propiedades de la luz ambiental
	XMVECTOR ambientLightColor = { 0.1f, 0.1f, 0.1f, 0.0f };
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
