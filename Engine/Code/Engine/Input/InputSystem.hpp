#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"

struct IntVec2;

constexpr int MAX_XBOX_CONTROLLERS = 4;


class InputSystem
{
public:
	InputSystem();
	~InputSystem();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void	UpdateMouse();
	IntVec2 GetMouseRawDesktopPosition() const;
	Vec2	GetMouseNormalizedClientPosition() const;

	void ResetKeys();
	void UpdateControllers();
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	const XboxController& GetXboxController( int controllerID );

	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;


private:
	Vec2 m_mouseNormalizedPos;
	KeyButtonState m_keyStates[ NUM_KEYS ];
	XboxController m_controllers[ MAX_XBOX_CONTROLLERS ] =
	{
		XboxController(0),
		XboxController(1),
		XboxController(2),
		XboxController(3)
	};
};