#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Platform/Window.hpp"

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
const unsigned char KEY_CODE_PLUS			= VK_OEM_PLUS;
const unsigned char KEY_CODE_MINUS			= VK_OEM_MINUS;
const unsigned char KEY_CODE_HOME			= VK_HOME;
const unsigned char KEY_CODE_END			= VK_END;

//Define Mouse Codes
const unsigned char MOUSE_CODE_LEFT			= MK_LBUTTON;
const unsigned char MOUSE_CODE_RIGHT		= MK_RBUTTON;
const unsigned char MOUSE_CODE_MIDDLE		= MK_MBUTTON;



//---------------------------------------------------------------------------------------------------------
InputSystem::InputSystem()
{
}


//---------------------------------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::StartUp( Window* theWindow )
{
	m_theWindow = theWindow;
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	UpdateMouse();
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

	for( int mouseIndex = 0; mouseIndex < NUM_MOUSE_BUTTONS; ++mouseIndex )
	{
		KeyButtonState& mouseState = m_mouseStates[ mouseIndex ];
		mouseState.m_wasPressedLastFrame = mouseState.m_isPressed;
	}

	m_scrollAmount = 0.f;
	m_characters = std::queue<char>();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::ShutDown()
{

}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateMouse()
{
	switch( m_mouseMode )
	{
	case MOUSE_MODE_ABSOLUTE:
		UpdateAbsoluteMode();
		break;
	case MOUSE_MODE_RELATIVE:
		UpdateRelativeMode();
		break;
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
IntVec2 InputSystem::GetMouseRawDesktopPosition() const
{
	POINT rawMouseDesktopPos;
	::GetCursorPos( &rawMouseDesktopPos );
	return IntVec2( rawMouseDesktopPos.x, rawMouseDesktopPos.y );
}


//---------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetMouseNormalizedClientPosition() const
{
	return m_mouseNormalizedPos;
}


//---------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorRelativeMovement() const
{
	return m_cursorRelativeMovement;
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::ShowSystemCursor( bool isShown )
{
	if( isShown )
	{
		while( ::ShowCursor( isShown ) < 0 ) {}
		return;
	}
	else
	{
		while( ::ShowCursor( isShown ) > 0 ) {}
		return;
	}
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::ClipSystemCursor( AABB2 const* windowDimensions )
{
	if( windowDimensions != nullptr )
	{
		RECT windowRect;
		windowRect.left		= (LONG)windowDimensions->mins.x;
		windowRect.bottom	= (LONG)windowDimensions->mins.y;
		windowRect.right	= (LONG)windowDimensions->maxes.x;
		windowRect.top		= (LONG)windowDimensions->maxes.y;

		if( ::ClipCursor( &windowRect ) )
			return;
	}
	::ClipCursor( NULL );
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::RecenterCursor()
{
	Vec2 windowCenter = m_theWindow->GetClientCenter();
	SetCursorPos( static_cast<int>( windowCenter.x ), static_cast<int>( windowCenter.y ) );

	//Eliminate Drift
	POINT point;
	GetCursorPos( &point );
	windowCenter = Vec2( static_cast<float>( point.x ), static_cast<float>( point.y ) );

	m_cursorPositionLastFrame = windowCenter;
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateRelativeMode()
{
	POINT point;
	::GetCursorPos( &point );
	Vec2 cursorPositionThisFrame = Vec2( static_cast<float>( point.x ), static_cast<float>( point.y ) );
	m_cursorRelativeMovement = cursorPositionThisFrame - m_cursorPositionLastFrame;
	m_cursorRelativeMovement *= -1.f;
	RecenterCursor();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateAbsoluteMode()
{
	POINT screenMousePos;
	::GetCursorPos( &screenMousePos );
	::ScreenToClient( (HWND)m_theWindow->m_hwnd, &screenMousePos );
	Vec2 mouseClientPos( static_cast<float>(screenMousePos.x), static_cast<float>(screenMousePos.y) );

	RECT clientRect;
	::GetClientRect( (HWND)m_theWindow->m_hwnd, &clientRect );
	AABB2 clientBounds( static_cast<float>(clientRect.left),
		static_cast<float>(clientRect.bottom),
		static_cast<float>(clientRect.right),
		static_cast<float>(clientRect.top) ); //Windows ( 0, 0 ) is top left

	m_mouseNormalizedPos = clientBounds.GetUVForPoint( mouseClientPos );
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::SetCursorMode( MousePositionMode mode )
{
	m_mouseMode = mode;
	switch( mode )
	{
	case MOUSE_MODE_ABSOLUTE:
	{
		m_isCursorLocked = false;
		ShowSystemCursor( true );
		break;
	}
	case MOUSE_MODE_RELATIVE:
	{
		m_isCursorLocked = true;
		ShowSystemCursor( false );
		RecenterCursor();
		break;
	}
	default:
		break;
	}
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
void InputSystem::ResetMouse()
{
	for( int mouseIndex = 0; mouseIndex < NUM_MOUSE_BUTTONS; ++mouseIndex )
	{
		KeyButtonState& mouseState = m_mouseStates[ mouseIndex ];
		mouseState.Reset();
	}
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::UpdateMouseButtonState( bool leftButton, bool rightButton, bool middleButton )
{
	m_mouseStates[ MOUSE_BUTTON_LEFT ].UpdateStatus( leftButton );
	m_mouseStates[ MOUSE_BUTTON_RIGHT ].UpdateStatus( rightButton );
	m_mouseStates[ MOUSE_BUTTON_MIDDLE ].UpdateStatus( middleButton );
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::IsMouseButtonPressed( MouseButtons mouseButton ) const
{
	return m_mouseStates[ mouseButton ].IsPressed();
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustPressed( MouseButtons mouseButton ) const
{
	return m_mouseStates[ mouseButton ].WasJustPressed();
}


//---------------------------------------------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustReleased( MouseButtons mouseButton ) const
{
	return m_mouseStates[ mouseButton ].WasJustReleased();
}


//---------------------------------------------------------------------------------------------------------
void InputSystem::AddMouseWheelScrollAmount( float scrollAmount )
{
	m_scrollAmount += scrollAmount;
}


//---------------------------------------------------------------------------------------------------------
float InputSystem::GetScrollAmount() const
{
	return m_scrollAmount;
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


//---------------------------------------------------------------------------------------------------------
void InputSystem::AddStringToClipboard( std::string stringToAdd )
{
	if( !OpenClipboard( nullptr ) ) return;
	if( !EmptyClipboard() ) return;

	HGLOBAL hGlob = GlobalAlloc( GMEM_FIXED, 64 );
	strcpy_s( (char*)hGlob, 64, stringToAdd.c_str() );
	SetClipboardData( CF_TEXT, hGlob );

	CloseClipboard();
}


//---------------------------------------------------------------------------------------------------------
std::string InputSystem::GetStringFromClipboard()
{
	std::string stringFromClipboard = ""; 

	if( !OpenClipboard( nullptr ) ) 
		return stringFromClipboard;

	stringFromClipboard = (char*)GetClipboardData( CF_TEXT );
	CloseClipboard();

	return stringFromClipboard;
}
