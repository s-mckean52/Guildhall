#pragma once

#define UNUSED(x) (void)(x);
#define STATIC
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/NamedStrings.hpp"

class DevConsole;
class EventSystem;

enum KeyboardKeys
{
	INVALID_KEY = -1,

	KEY_0 = 0x30,

	KEY_A = 0x41,

	KEY_OEM_CLEAR = 0xFE,

	NUM_KEYS
};

extern DevConsole* g_theConsole;
extern EventSystem* g_theEventSystem;
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
extern const unsigned char KEY_CODE_TILDE;


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
