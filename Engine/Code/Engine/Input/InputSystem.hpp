#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <queue>

enum MousePositionMode
{
	MOUSE_MODE_ABSOLUTE,
	MOUSE_MODE_RELATIVE,
};

struct IntVec2;
struct AABB2;
class Window;

constexpr int MAX_XBOX_CONTROLLERS = 4;

class InputSystem
{
public:
	InputSystem();
	~InputSystem();

	void StartUp( Window* theWindow );
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	// Mouse
	void	UpdateMouse();
	IntVec2 GetMouseRawDesktopPosition() const;
	Vec2	GetMouseNormalizedClientPosition() const;
	Vec2	GetCursorRelativeMovement() const;

	void ShowSystemCursor( bool isShown );
	void ClipSystemCursor( AABB2 const* windowDimensions );
	void RecenterCursor();
	void UpdateRelativeMode();
	void UpdateAbsoluteMode();
	void SetCursorMode( MousePositionMode mode );

	// Keyboard
	void ResetKeys();
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;

	void PushToCharacterQueue( char c );
	bool PopFromCharacterQueue( char* c );

	void		AddStringToClipboard( std::string stringToAdd );
	std::string	GetStringFromClipboard();

	// Controllers
	void UpdateControllers();
	const XboxController& GetXboxController( int controllerID );

private:
	Window* m_theWindow = nullptr;

	MousePositionMode m_mouseMode = MOUSE_MODE_ABSOLUTE;
	Vec2 m_cursorPositionLastFrame;
	Vec2 m_cursorRelativeMovement;
	Vec2 m_mouseNormalizedPos;
	bool m_isCursorLocked = false;

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