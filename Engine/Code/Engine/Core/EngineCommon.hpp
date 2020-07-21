#pragma once

#define STATIC
#define UNUSED(x) (void)(x);
//#define BIT_FLAG(b)	( 1 << (b) );
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/Vec2.hpp"


typedef unsigned int uint;

struct Mat44;
class DevConsole;
class EventSystem;
class JobSystem;

enum KeyboardKeys
{
	INVALID_KEY = -1,

	KEY_0 = 0x30,

	KEY_A = 0x41,

	KEY_OEM_CLEAR = 0xFE,

	NUM_KEYS
};

enum MouseButtons
{
	INVALID_BUTTON = -1,
	
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,

	NUM_MOUSE_BUTTONS
};

extern float		SQRT_2_OVER_2;
extern DevConsole*	g_theConsole;
extern EventSystem* g_theEventSystem;
extern JobSystem*	g_theJobSystem;
extern NamedStrings g_gameConfigBlackboard;

//Key Codes
extern const unsigned char KEY_CODE_ESC;
extern const unsigned char KEY_CODE_ENTER;
extern const unsigned char KEY_CODE_SPACEBAR;
extern const unsigned char KEY_CODE_BACKSPACE;
extern const unsigned char KEY_CODE_DELETE;
extern const unsigned char KEY_CODE_UP_ARROW;
extern const unsigned char KEY_CODE_LEFT_ARROW;
extern const unsigned char KEY_CODE_DOWN_ARROW;
extern const unsigned char KEY_CODE_RIGHT_ARROW;
extern const unsigned char KEY_CODE_SHIFT;
extern const unsigned char KEY_CODE_CTRL;
extern const unsigned char KEY_CODE_COPY;
extern const unsigned char KEY_CODE_PASTE;
extern const unsigned char KEY_CODE_CUT;
extern const unsigned char KEY_CODE_F1;
extern const unsigned char KEY_CODE_F2;
extern const unsigned char KEY_CODE_F3;
extern const unsigned char KEY_CODE_F4;
extern const unsigned char KEY_CODE_F5;
extern const unsigned char KEY_CODE_F6;
extern const unsigned char KEY_CODE_F7;
extern const unsigned char KEY_CODE_F8;
extern const unsigned char KEY_CODE_F9;
extern const unsigned char KEY_CODE_F10;
extern const unsigned char KEY_CODE_F11;
extern const unsigned char KEY_CODE_F12;
extern const unsigned char KEY_CODE_PLUS;
extern const unsigned char KEY_CODE_MINUS;
extern const unsigned char KEY_CODE_HOME;
extern const unsigned char KEY_CODE_END;
extern const unsigned char KEY_CODE_TILDE;
extern const unsigned char KEY_CODE_LEFT_BRACKET;
extern const unsigned char KEY_CODE_RIGHT_BRACKET;
extern const unsigned char KEY_CODE_COMMA;
extern const unsigned char KEY_CODE_PERIOD;
extern const unsigned char KEY_CODE_SEMICOLON;
extern const unsigned char KEY_CODE_BACK_SLASH;
extern const unsigned char KEY_CODE_APOSTROPHE;

extern const unsigned char MOUSE_CODE_LEFT;
extern const unsigned char MOUSE_CODE_RIGHT;
extern const unsigned char MOUSE_CODE_MIDDLE;



//---------------------------------------------------------------------------------------------------------
const Vec2 ALIGN_TOP_CENTERED		= Vec2( 0.5f, 1.f );
const Vec2 ALIGN_CENTERED			= Vec2( 0.5f, 0.5f );
const Vec2 ALIGN_BOTTOM_CENTERED	= Vec2( 0.5f, 0.f );

const Vec2 ALIGN_TOP_LEFT			= Vec2( 0.f, 1.f );
const Vec2 ALIGN_CENTER_LEFT		= Vec2( 0.f, 0.5f );
const Vec2 ALIGN_BOTTOM_LEFT		= Vec2( 0.f, 0.f );

const Vec2 ALIGN_TOP_RIGHT			= Vec2( 1.f, 1.f );
const Vec2 ALIGN_CENTER_RIGHT		= Vec2( 1.f, 0.5f );
const Vec2 ALIGN_BOTTOM_RIGHT		= Vec2( 1.f, 0.f );



//---------------------------------------------------------------------------------------------------------
const unsigned int MAX_LIGHTS = 8;