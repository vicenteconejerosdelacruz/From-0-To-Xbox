#pragma once

#include "pch.h"
#include "Renderer/Renderer.h"
#include "Primitives/Cube.h"
#include "2D/FPSText.h"
#include "2D/XBoxOneButton.h"
#include "2D/XBoxOneViewButton.h"
#include "2D/KeyboardButton.h"
#include "2D/Label.h"
#include "Common/StepTimer.h"

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

	//FPS
	std::shared_ptr<FPSText> fpsText;
	DX::StepTimer timer;

	//GamePad
	std::unique_ptr<DirectX::GamePad> gamePad;
	DirectX::GamePad::ButtonStateTracker buttons;
	std::shared_ptr<XBoxOneButton> xboxOneButtonA;
	std::shared_ptr<XBoxOneButton> xboxOneButtonB;
	std::shared_ptr<XBoxOneButton> xboxOneButtonX;
	std::shared_ptr<XBoxOneButton> xboxOneButtonY;
	std::shared_ptr<XBoxOneViewButton> xboxOneViewButton;

	//Keyboard
	std::unique_ptr<DirectX::Keyboard> keyboard;
	DirectX::Keyboard::KeyboardStateTracker keys;
	std::shared_ptr<KeyboardButton> keyboardButtonA;
	std::shared_ptr<KeyboardButton> keyboardButtonS;
	std::shared_ptr<KeyboardButton> keyboardButtonD;
	std::shared_ptr<KeyboardButton> keyboardButtonP;
	std::shared_ptr<KeyboardButton> keyboardButtonL;

	//Light status labels
	std::shared_ptr<Label> ambientLightLabel;
	std::shared_ptr<Label> directionalLightLabel;
	std::shared_ptr<Label> spotLightLabel;
	std::shared_ptr<Label> pointLightLabel;
	std::shared_ptr<Label> lightModelLabelCentered;
	std::shared_ptr<Label> lightModelLabelLeft;

	//camera properties
	//propiedades de la camara
	XMVECTOR	cameraPos = { 0.0f, 0.0f, -5.5f, 0.0f };
	XMVECTOR	cameraFw = { 0.0f, 0.0f, 1.0f, 0.0f };
	XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };

	/*AMBIENT, DIRECTIONAL, SPOT, POINT*/
	BOOL lightsEnabled[4] = { TRUE, TRUE, TRUE, TRUE };
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
