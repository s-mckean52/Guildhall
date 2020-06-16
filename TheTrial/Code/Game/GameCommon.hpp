#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define UNUSED(x) (void)(x);

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

constexpr int MAX_ABILITY_COUNT = 4;
constexpr float ABILITY_UI_WIDTH = 1.f;
constexpr float ABILITY_UI_HEIGHT = 1.f;

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