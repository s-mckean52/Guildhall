#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static TCHAR const* WND_CLASS_NAME = L"Simple Window Class";


//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
static LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	Window* window = (Window*) ::GetWindowLongPtr( windowHandle, GWLP_USERDATA );

	switch( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			window->SetIsQuitting( true );
			return 0;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;
			
			InputSystem* theInput = window->GetInputSystem();
			return theInput->HandleKeyPressed( asKey );

			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)wParam;

			InputSystem* theInput = window->GetInputSystem();
			return theInput->HandleKeyReleased( asKey );

			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			bool leftButtonDown		= wParam & MOUSE_CODE_LEFT;
			bool rightButtonDown	= wParam & MOUSE_CODE_RIGHT;
			bool middleButtonDown	= wParam & MOUSE_CODE_MIDDLE;

			InputSystem* theInput = window->GetInputSystem();
			theInput->UpdateMouseButtonState( leftButtonDown, rightButtonDown, middleButtonDown );

			break;
		}

		case WM_MOUSEWHEEL:
		{
			float scrollFixedPoint = static_cast<float>(GET_WHEEL_DELTA_WPARAM( wParam ));
			float scrollAmount = scrollFixedPoint / static_cast<float>(WHEEL_DELTA);

			InputSystem* theInput = window->GetInputSystem();
			theInput->AddMouseWheelScrollAmount( scrollAmount );

			break;
		}

		case WM_CHAR:
		{
			wchar_t inputCharacter = (wchar_t)wParam;
			InputSystem* theInput = window->GetInputSystem();
			theInput->PushToCharacterQueue( (char)inputCharacter );
			break;
		}

		case WM_ACTIVATE:
		{
			RECT windowRect;
			::GetWindowRect( windowHandle, &windowRect );

			AABB2 windowDimensions = AABB2(	static_cast<float>( windowRect.left ),
											static_cast<float>( windowRect.bottom ), 
											static_cast<float>( windowRect.right ),
											static_cast<float>( windowRect.top ) );

			InputSystem* theInput = window->GetInputSystem();
			EventSystem* theEventSystem = window->GetEventSystem();
			if( wParam == WA_ACTIVE )
			{
				if( theInput ) theInput->ClipSystemCursor( &windowDimensions );
				if( theEventSystem) theEventSystem->FireEvent( "GainFocus" );
			}
			else if( wParam == WA_INACTIVE )
			{
				if( theInput) theInput->ClipSystemCursor( nullptr );
				if( theEventSystem) theEventSystem->FireEvent( "LoseFocus" );
			}
			break;
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return ::DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//---------------------------------------------------------------------------------------------------------
static void RegisterWindowClass()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = WND_CLASS_NAME;
	::RegisterClassEx( &windowClassDescription );
}


//---------------------------------------------------------------------------------------------------------
static void UnregisterWindowClass()
{
	::UnregisterClass( WND_CLASS_NAME, GetModuleHandle( NULL ) );
}


//---------------------------------------------------------------------------------------------------------
Window::Window()
	: m_hwnd( nullptr )
{
	RegisterWindowClass();
}


//---------------------------------------------------------------------------------------------------------
Window::~Window()
{
	Close();
	UnregisterWindowClass();
}


//---------------------------------------------------------------------------------------------------------
bool Window::Open( std::string const& title, float clientAspect, float ratioOfHeight )
{
	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	float clientWidth = desktopWidth * ratioOfHeight;
	float clientHeight = desktopHeight * ratioOfHeight;
	if( clientAspect > desktopAspect )
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, title.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		WND_CLASS_NAME,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE) ::GetModuleHandle( NULL ),
		NULL );

	::SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)this );

	if( hwnd == nullptr )
	{
		return false;
	}

	ShowWindow( hwnd, SW_SHOW );
	SetForegroundWindow( hwnd );
	SetFocus( hwnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	m_hwnd = (void*)hwnd;

	m_height = static_cast<unsigned int>( clientHeight );
	m_width = static_cast<unsigned int>( clientWidth );

	return true;
}


//---------------------------------------------------------------------------------------------------------
void Window::Close()
{
	HWND hwnd = (HWND)m_hwnd;
	if( NULL == hwnd )
	{
		return;
	}

	::DestroyWindow( hwnd );
	m_hwnd = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Window::SetInputSystem( InputSystem* input )
{
	m_theInput = input;
	AABB2 windowBounds = GetBoundsAsAABB2();
	input->ClipSystemCursor( &windowBounds );
}


//---------------------------------------------------------------------------------------------------------
void Window::SetEventSystem( EventSystem* eventSystem )
{
	m_theEventSystem = eventSystem;
}


//---------------------------------------------------------------------------------------------------------
void Window::SetIsQuitting( bool isQuitting )
{
	m_isQuitting = isQuitting;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Window::GetClientCenter() const
{
	RECT clientRect;
	::GetClientRect( (HWND)m_hwnd, &clientRect );

	float width = static_cast<float>( clientRect.right - clientRect.left );
	float height = static_cast<float>( clientRect.top - clientRect.bottom );

	float horizontalCenter = clientRect.left + ( width * 0.5f );
	float verticalCenter = clientRect.bottom + ( height * 0.5f );

	return Vec2( horizontalCenter, verticalCenter );
}


//---------------------------------------------------------------------------------------------------------
AABB2 Window::GetBoundsAsAABB2() const
{
	HWND hwnd = (HWND)m_hwnd;
	if( hwnd == NULL )
	{
		ERROR_AND_DIE( "Tried to get bounds of a nonexistent window" );
	}

	RECT windowRect;
	::GetWindowRect( hwnd, &windowRect );
	AABB2 windowDimensions = AABB2(	static_cast<float>( windowRect.left ),
									static_cast<float>( windowRect.bottom ), 
									static_cast<float>( windowRect.right ),
									static_cast<float>( windowRect.top ) );

	return windowDimensions;
}


//---------------------------------------------------------------------------------------------------------
void Window::BeginFrame()
{
	//-----------------------------------------------------------------------------------------------
	// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
	// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
	//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
	//
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}