#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"

//extern HWND g_hWnd;


//Define Key Codes
const unsigned char KEY_CODE_ESC			= VK_ESCAPE;
const unsigned char KEY_CODE_ENTER			= VK_RETURN;
const unsigned char KEY_CODE_SPACEBAR		= VK_SPACE;
const unsigned char KEY_CODE_BACKSPACE		= VK_BACK;
const unsigned char KEY_CODE_DELETE			= VK_DELETE;
const unsigned char KEY_CODE_UP_ARROW		= VK_UP;
const unsigned char KEY_CODE_LEFT_ARROW		= VK_LEFT;
const unsigned char KEY_CODE_DOWN_ARROW		= VK_DOWN;
const unsigned char KEY_CODE_RIGHT_ARROW	= VK_RIGHT;
const unsigned char KEY_CODE_SHIFT			= VK_SHIFT;
const unsigned char KEY_CODE_CTRL			= VK_CONTROL;
const unsigned char KEY_CODE_COPY			= 0x03;
const unsigned char KEY_CODE_PASTE			= 0x16;
const unsigned char KEY_CODE_CUT			= 0x18;
const unsigned char KEY_CODE_F1				= VK_F1;
const unsigned char KEY_CODE_F2				= VK_F2;
const unsigned char KEY_CODE_F3				= VK_F3;
const unsigned char KEY_CODE_F4				= VK_F4;
const unsigned char KEY_CODE_F5				= VK_F5;
const unsigned char KEY_CODE_F6				= VK_F6;
const unsigned char KEY_CODE_F7				= VK_F7;
const unsigned char KEY_CODE_F8				= VK_F8;
const unsigned char KEY_CODE_F9				= VK_F9;
const unsigned char KEY_CODE_F10			= VK_F10;
const unsigned char KEY_CODE_F11			= VK_F11;
const unsigned char KEY_CODE_F12			= VK_F12;
const unsigned char KEY_CODE_TILDE			= VK_OEM_3;
const unsigned char KEY_CODE_HOME			= VK_HOME;
const unsigned char KEY_CODE_END			= VK_END;


//---------------------------------------------------------------------------------------------------------
InputSystem::InputSystem()
{
}


//---------------------------------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::StartUp()
{

}


//---------------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	//UpdateMouse();
	UpdateControllers();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
	for( int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex )
	{
		KeyButtonState& keyState = m_keyStates[ keyIndex ];
		keyState.m_wasPressedLastFrame = keyState.m_isPressed;
	}

	m_characters = std::queue<char>();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::ShutDown()
{

}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateMouse()
{
// 	POINT screenMousePos;
// 	GetCursorPos( &screenMousePos );
// 	ScreenToClient( g_hWnd, &screenMousePos );
// 	Vec2 mouseClientPos( static_cast<float>(screenMousePos.x), static_cast<float>(screenMousePos.y) );
// 
// 	RECT clientRect;
// 	GetClientRect( g_hWnd, &clientRect );
// 	AABB2 clientBounds( static_cast<float>( clientRect.left ),
// 						static_cast<float>( clientRect.bottom ),
// 						static_cast<float>( clientRect.right ),
// 						static_cast<float>( clientRect.top ) ); //Windows ( 0, 0 ) is top left
// 
// 	m_mouseNormalizedPos = clientBounds.GetUVForPoint( mouseClientPos );
	UNIMPLEMENTED_MSG( "Mouse input to not use hwnd" );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 InputSystem::GetMouseRawDesktopPosition() const
{
	POINT rawMouseDesktopPos;
	GetCursorPos( &rawMouseDesktopPos );
	return IntVec2( rawMouseDesktopPos.x, rawMouseDesktopPos.y );
}


//---------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetMouseNormalizedClientPosition() const
{
	return m_mouseNormalizedPos;
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::ResetKeys()
{
	for( int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex )
	{
		KeyButtonState& keyState = m_keyStates[ keyIndex ];
		keyState.Reset();
	}
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateControllers()
{
	for( int controllerIndex = 0; controllerIndex < MAX_XBOX_CONTROLLERS; ++controllerIndex )
	{
		m_controllers[controllerIndex].Update();
	}
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::HandleKeyPressed( unsigned char keyCode )
{
	m_keyStates[ keyCode ].m_isPressed = true;
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::HandleKeyReleased( unsigned char keyCode )
{
	m_keyStates[ keyCode ].m_isPressed = false;
	return true;
}


//---------------------------------------------------------------------------------------------------------
const XboxController& InputSystem::GetXboxController( int controllerID )
{
	return m_controllers[ controllerID ];
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyPressed( unsigned char keyCode ) const
{
	return m_keyStates[ keyCode ].m_isPressed;
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed( unsigned char keyCode ) const
{
	return m_keyStates[ keyCode ].WasJustPressed();
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased( unsigned char keyCode ) const
{
	return m_keyStates[ keyCode ].WasJustReleased();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::PushToCharacterQueue( char c )
{
	m_characters.push( c );
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::PopFromCharacterQueue( char* out_c )
{
	if( m_characters.size() > 0 )
	{
		*out_c = m_characters.front();
		m_characters.pop();
		return true;
	}
	else
	{
		return false;
	}
}

