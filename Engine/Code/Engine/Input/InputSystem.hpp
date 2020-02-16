#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <queue>

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
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	void UpdateControllers();
	const XboxController& GetXboxController( int controllerID );

	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;

	void PushToCharacterQueue( char c );
	bool PopFromCharacterQueue( char* c );

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

	std::queue<char> m_characters;
};