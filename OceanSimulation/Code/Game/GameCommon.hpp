#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include <complex>

struct Vec2;
class App;
class Game;
class RenderContext;
class RandomNumberGenerator;
class InputSystem;
class AudioSystem;
class BitmapFont;
class SpriteSheet;
class EventSystem;
class Window;
struct AABB2;


extern EventSystem* g_theEventSystem;
extern App* g_theApp;
extern Game* g_theGame;
extern RenderContext* g_theRenderer;
extern RandomNumberGenerator* g_RNG;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern BitmapFont* g_devConsoleFont;
extern Window* g_theWindow;

extern SpriteSheet* g_tileSpriteSheet;
extern SpriteSheet* g_actorSpriteSheet;

extern bool g_isDebugDraw;
extern bool g_isDebugCamera;

//---------------------------------------------------------------------------------------------------------
// Global Game-specific Constants
constexpr float GRAVITY					= 9.81f;
constexpr float INVERSE_GRAVITY			= 1.f / GRAVITY;
constexpr float MAX_FRAME_TIME			= .1f;
constexpr float CAMERA_SIZE_X			= 16.f;
constexpr float CAMERA_SIZE_Y			= 9.f;
constexpr float CLIENT_ASPECT			= CAMERA_SIZE_X / CAMERA_SIZE_Y;
constexpr float HALF_SCREEN_X			= CAMERA_SIZE_X * 0.5f;
constexpr float HALF_SCREEN_Y			= CAMERA_SIZE_Y * 0.5f;
constexpr int	MAX_NUM_PLAYERS			= 1;
constexpr float RAYCAST_STEP_PER_UNIT	= 100.f;
constexpr float RAYCAST_STEP_DISTANCE	= 1.f / RAYCAST_STEP_PER_UNIT;
constexpr float DEBUG_THICKNESS			= 0.02f;

constexpr float TRIGGER_ACTIVATION_FRACTION	= 0.6f;

constexpr float SCREEN_SHAKE_ABBERATION				= 1.f / 2.f;
constexpr float MAX_SCREEN_SHAKE_DISPLACEMENT		= 1.0f;
constexpr float PLAYER_DEATH_SCREEN_SHAKE_INTENSITY = 1.f;

constexpr float DEV_CONSOLE_LINE_HEIGHT				= 0.15f;

// Game Specific Colors
const Rgba8 RGBA8_BALI_HAI				( 132, 156, 169 ); //Teal Grey
const Rgba8 RGBA8_DARK_GREY				( 50, 50, 50 );
const Rgba8 RGBA8_MEDIUM_DARK_GREY		( 100, 100, 100 );
const Rgba8 RGBA8_TRANSPARENT_RED		( 255, 0, 0, 0 );

const Rgba8 RGBA8_DARK_GREEN			( 0, 150, 0 );
const Rgba8 RGBA8_DARK_YELLOW			( 200, 170, 0 );
const Rgba8 RGBA8_HALF_TRANSPARENT_GRAY	( 127, 127, 127, 127 );

// Common Game Functions
void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness );
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8& color, float thickness );
void DrawAABB2AtPoint( const Vec2& position, const AABB2& box, const Rgba8& color, float thickness );



//---------------------------------------------------------------------------------------------------------
// Key Bindings
//---------------------------------------------------------------------------------------------------------
// Camera
const float CAMERA_MOVE_SPEED			= 5.f;
const float CAMERA_FAST_FACTOR			= 2.f;
const unsigned char CAMERA_FORWARD_KEY	= 'W';
const unsigned char CAMERA_BACKWARD_KEY	= 'S';
const unsigned char CAMERA_LEFT_KEY		= 'A';
const unsigned char CAMERA_RIGHT_KEY	= 'D';
const unsigned char CAMERA_UP_KEY		= 'Q';
const unsigned char CAMERA_DOWN_KEY		= 'E';
const unsigned char CAMERA_FAST_KEY		= KEY_CODE_SHIFT;

//Crate
const float BOX_MOVEMENT_SPEED			= 2.f;
const unsigned char BOX_FORWARD_KEY		= KEY_CODE_UP_ARROW;
const unsigned char BOX_BACKWARD_KEY	= KEY_CODE_DOWN_ARROW;
const unsigned char BOX_LEFT_KEY		= KEY_CODE_LEFT_ARROW;
const unsigned char BOX_RIGHT_KEY		= KEY_CODE_RIGHT_ARROW;

//Toggles
const unsigned char TOGGLE_IWAVE_KEY = 'I';
const unsigned char TOGGLE_UNDERWATER_EFFECT_KEY = 'U';
const unsigned char TOGGLE_SCROLLING_NORMALS_KEY = '5';
const unsigned char APPLY_IWAVE_SOURCES_KEY = '4';
const float			IWAVE_SOURCE_POWER_PER_SCROLL = 0.5f;
const unsigned char TOGGLE_CRATE_KEY = '3';
const unsigned char TOGGLE_SKYBOX_KEY = KEY_CODE_F3;

//Cycles
const unsigned char CYCLE_WATER_SHADERS_FORWARD = 'R';
const unsigned char CYCLE_WATER_SHADERS_BACKWARD = 'F';

const unsigned char CYCLE_TERRAIN_FORWARD = 'X';
const unsigned char CYCLE_TERRAIN_BACKWARD = 'Z';

const unsigned char CYCLE_SETTINGS_FORWARD = '2';
const unsigned char CYCLE_SETTINGS_BACKWARD = '1';

//Updates
const unsigned char SELECT_NUM_TILES = 'N';
const unsigned int	NUM_TILES_CHANGE = 1;

const unsigned char SELECT_TIME_FACTOR = 'T';
const unsigned int	TIME_FACTOR_CHANGE = 1;

const unsigned char	SELECT_CHOPPINES = 'C';
const float			CHOPINESS_CHANGE_PER_SCROLL = 0.1f;

const unsigned char SELECT_SAMPLES = 'Y';

const unsigned char SELECT_DIMENSIONS = 'G';
const float			DIMENSIONS_PER_SCROLL = 0.5f;

const unsigned char SELECT_GLOBAL_AMPLITUDE = 'H';
const float			GLOBAL_AMPLITUDE_PER_SCROLL = 0.0005f;

const unsigned char SELECT_WAVE_SUPPRESS = 'J';
const float			WAVE_SUPPRESS_PER_SCROLL = 0.0005f;

const unsigned char SELECT_WIND_SPEED = 'K';
const float			WIND_SPEED_PER_SCROLL = 0.33f;

const unsigned char SELECT_WIND_DIRECTION = 'L';
const float			WIND_ROTATION_PER_SCROLL = 5.f;

const unsigned char SELECT_FOAM_DEPTH = 'V';
const float			FOAM_DEPTH_PER_SCROLL = 0.1f;

const unsigned char SELECT_FOAMINESS = 'B';
const float			FOAMINESS_PER_SCROLL = 0.1f;

const unsigned char SELECT_MAX_DEPTH = 'M';
const float			MAX_DEPTH_PER_SCROLL = 1.f;