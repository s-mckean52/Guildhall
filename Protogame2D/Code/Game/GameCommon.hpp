#pragma once
#include "Engine/Core/Rgba8.hpp"

#define UNUSED(x) (void)(x);

class App;
class Game;
class RenderContext;
struct Vec2;
class RandomNumberGenerator;
class InputSystem;

extern App *g_theApp;
extern Game *g_theGame;
extern RenderContext *g_theRenderer;
extern RandomNumberGenerator *g_RNG;
extern InputSystem *g_theInput;

extern bool g_isDebugCamera;
extern bool g_isDebugDraw;
extern bool g_isNoClip;


// Global Game-specific Constants
constexpr float MAX_FRAME_TIME		= .1f;
constexpr float CAMERA_SIZE_X		= 16.f;
constexpr float CAMERA_SIZE_Y		= 9.f;
constexpr float CLIENT_ASPECT		= CAMERA_SIZE_X / CAMERA_SIZE_Y;
constexpr float HALF_SCREEN_X		= CAMERA_SIZE_X * 0.5f;
constexpr float HALF_SCREEN_Y		= CAMERA_SIZE_Y * 0.5f;
constexpr int	MAX_NUM_PLAYERS		= 2;
constexpr int	MAX_PLAYER_LIVES	= 4;
constexpr float RESTART_DELAY		= 3.f;
constexpr int	MAP_SIZE_X			= 20;
constexpr int	MAP_SIZE_Y			= 30;
constexpr int	NUM_MAPS			= 1;
constexpr float TILE_SIZE			= 1.f;

constexpr float TRIGGER_ACTIVATION_FRACTION	= 0.6f;

constexpr float SCREEN_SHAKE_ABBERATION				= 1.f / 2.f;
constexpr float MAX_SCREEN_SHAKE_DISPLACEMENT		= 1.0f;
constexpr float PLAYER_DEATH_SCREEN_SHAKE_INTENSITY = 1.f;

constexpr float	PLAYER_ACCELERATION					= 20.f;
constexpr float PLAYER_MAX_SPEED					= 1.f;
constexpr float PLAYER_FRICTION_PER_SECOND			= 11.f;
constexpr float PLAYER_PHYSICS_RADIUS				= 0.3f;
constexpr float PLAYER_COSMETIC_RADIUS				= 0.4f;
constexpr float DEBUG_THICKNESS						= 0.02f;
constexpr float PLAYER_ANGULAR_VELOCITY				= 180.f;

// Colors
const Rgba8 RGBA8_BLACK				( 0, 0, 0 );
const Rgba8 RGBA8_WHITE				( 255, 255, 255 );
const Rgba8 RGBA8_RED				( 255, 0, 0 );
const Rgba8 RGBA8_GREEN				( 0, 255, 0 );
const Rgba8 RGBA8_BLUE				( 0, 0, 255 );

const Rgba8 RGBA8_YELLOW			( 255, 255, 0 );
const Rgba8 RGBA8_MAGENTA			( 255, 0, 255 );
const Rgba8 RGBA8_CYAN				( 0, 255, 255 );

const Rgba8 RGBA8_BALI_HAI			( 132, 156, 169 ); //Teal Grey
const Rgba8 RGBA8_DARK_GREY			( 50, 50, 50 );
const Rgba8 RGBA8_MEDIUM_DARK_GREY	( 100, 100, 100 );
const Rgba8 RGBA8_TRANSPARENT_RED	( 255, 0, 0, 0 );

const Rgba8 RGBA8_DARK_GREEN		( 0, 150, 0 );
const Rgba8 RGBA8_DARK_YELLOW		( 200, 170, 0 );


void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness );
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8 color, float thickness );