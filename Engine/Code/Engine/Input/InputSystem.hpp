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

	void ResetMouse();
	void UpdateMouseButtonState( bool leftButton, bool rightButton, bool middleButton );
	bool IsMouseButtonPressed( MouseButtons mouseButton = MOUSE_BUTTON_LEFT ) const;
	bool WasMouseButtonJustPressed( MouseButtons mouseButton = MOUSE_BUTTON_LEFT ) const;
	bool WasMouseButtonJustReleased( MouseButtons mouseButton = MOUSE_BUTTON_LEFT ) const;

	void	AddMouseWheelScrollAmount( float scrollAmount );
	float	GetScrollAmount() const;

private:
	KeyButtonState m_keyStates[ NUM_KEYS ];
	
	float			m_scrollAmount = 0.f;
	Vec2			m_mouseNormalizedPos;
	KeyButtonState	m_mouseStates[ NUM_MOUSE_BUTTONS ];

	XboxController m_controllers[ MAX_XBOX_CONTROLLERS ] =
	{
		XboxController(0),
		XboxController(1),
		XboxController(2),
		XboxController(3)
	};
};