#pragma once

#include "pch.h"
#include "Renderer/Renderer.h"
#include "Primitives/Cube.h"
#include "Primitives/Pyramid.h"
#include "Primitives/Floor.h"
#include "3D/Static3DModel.h"
#include "3D/Animated3DModel.h"
#include "2D/FPSText.h"
#include "2D/XBoxOneButton.h"
#include "2D/XBoxOneViewButton.h"
#include "2D/XBoxOneTrigger.h"
#include "2D/XBoxOneBumper.h"
#include "2D/XBoxOneDPad.h"
#include "2D/KeyboardButton.h"
#include "2D/Label.h"
#include "Lights/DirectionalLight.h"
#include "Lights/SpotLight.h"
#include "Lights/PointLight.h"
#include "Lights/AmbientLight.h"
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
	std::shared_ptr<Pyramid> pyramid;
	std::shared_ptr<Floor> floor;
	std::shared_ptr<Static3DModel> scene;
	std::shared_ptr<Animated3DModel> knight;

	//FPS
	std::shared_ptr<FPSText> fpsText;
	DX::StepTimer timer;

	//Mouse
	std::unique_ptr<DirectX::Mouse> mouse;
	XMFLOAT2 lastMousePos;

	//GamePad
	std::unique_ptr<DirectX::GamePad> gamePad;
	DirectX::GamePad::ButtonStateTracker buttons;
	std::shared_ptr<XBoxOneButton> xboxOneButtonA;
	std::shared_ptr<XBoxOneButton> xboxOneButtonB;
	std::shared_ptr<XBoxOneButton> xboxOneButtonX;
	std::shared_ptr<XBoxOneButton> xboxOneButtonY;
	std::shared_ptr<XBoxOneViewButton> xboxOneViewButton;
	std::shared_ptr<XBoxOneTrigger> xboxOneButtonLT;
	std::shared_ptr<XBoxOneTrigger> xboxOneButtonRT;
	std::shared_ptr<XBoxOneBumper> xboxOneButtonLB;
	std::shared_ptr<XBoxOneBumper> xboxOneButtonRB;

	//Keyboard
	std::unique_ptr<DirectX::Keyboard> keyboard;
	DirectX::Keyboard::KeyboardStateTracker keys;
	std::shared_ptr<KeyboardButton> keyboardButtonA;
	std::shared_ptr<KeyboardButton> keyboardButtonS;
	std::shared_ptr<KeyboardButton> keyboardButtonD;
	std::shared_ptr<KeyboardButton> keyboardButtonP;
	std::shared_ptr<KeyboardButton> keyboardButtonL;
	std::shared_ptr<KeyboardButton> keyboardButtonM;
	std::shared_ptr<KeyboardButton> keyboardButtonN;
	std::shared_ptr<KeyboardButton> keyboardButtonPU;
	std::shared_ptr<KeyboardButton> keyboardButtonPD;
	std::shared_ptr<KeyboardButton> keyboardButtonPlus;
	std::shared_ptr<KeyboardButton> keyboardButtonMinus;

	//Light status labels
	std::shared_ptr<Label> ambientLightLabel;
	std::shared_ptr<Label> directionalLightLabel;
	std::shared_ptr<Label> spotLightLabel;
	std::shared_ptr<Label> pointLightLabel;
	std::shared_ptr<Label> lightModelLabelCentered;
	std::shared_ptr<Label> lightModelLabelLeft;
	std::shared_ptr<Label> shadowMapsLabel;
	std::shared_ptr<Label> normalMappingLabel;

	//camera selection
	INT currentPerspective = 0;
	std::shared_ptr<Label> perspectiveSelectionLabel;
	std::shared_ptr<Label> perspectiveSelectionLabelPrevious;
	std::shared_ptr<Label> perspectiveSelectionLabelNext;

	//animation
	std::shared_ptr<Label> animationLabel;
	std::shared_ptr<Label> nextAnimationLabel;
	std::shared_ptr<Label> previousAnimationLabel;
	std::shared_ptr<XBoxOneDPad> xboxOneDPad;

	//camera properties
	//propiedades de la camara
	float cameraSpeed = 0.05f;
	XMVECTOR	cameraPos = { -5.7f, 2.2f, 3.8f , 0.0f };
	//yaw & pitch
	XMFLOAT2	cameraRotations = { DirectX::XM_PIDIV2 , -DirectX::XM_PIDIV4*0.25f };
	XMVECTOR	cameraFw() {
		return {
			sinf(cameraRotations.x) * cosf(cameraRotations.y),
			sinf(cameraRotations.y),
			cosf(cameraRotations.x) * cosf(cameraRotations.y)
		};
	}
	XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMVECTOR	right = { 1.0f, 0.0f, 0.0f, 0.0f };

	/*AMBIENT, DIRECTIONAL, SPOT, POINT*/
	BOOL lightsEnabled[4] = { TRUE, TRUE, TRUE, TRUE };
	/*Blinn-Phong:true*/
	BOOL useBlinnPhong = TRUE;
	/*SHADOW MAPS*/
	BOOL shadowMapsEnabled = TRUE;
	/*NORMAL MAPPING*/
	BOOL normalMappingEnabled = TRUE;

	//lighting
	//iluminacion
	std::shared_ptr<DirectionalLight> directionalLight;
	std::shared_ptr<SpotLight> spotLight;
	std::shared_ptr<PointLight> pointLight;
	std::shared_ptr<AmbientLight> ambientLight;
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
