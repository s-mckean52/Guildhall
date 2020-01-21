#pragma once
#include "Engine/Core/Rgba8.hpp"

#define UNUSED(x) (void)(x);

enum GameState
{
	INVALID_GAME_STATE = -1,

	ATTRACT_STATE,
	PLAY_STATE,
	SLOW_MO_STATE,
	PAUSED_STATE,
	DEBUG_STATE,
	RESTARTING_STATE,
	QUITTING_STATE,

	NUMBER_OF_STATES
};

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

// Global Game-specific Constants
constexpr float CLIENT_ASPECT		= 2.f;
constexpr float MAX_FRAME_TIME		= .1f;
constexpr float CAMERA_SIZE_X		= 200.f;
constexpr float CAMERA_SIZE_Y		= 100.f;
constexpr float HALF_SCREEN_X		= CAMERA_SIZE_X * 0.5f;
constexpr float HALF_SCREEN_Y		= CAMERA_SIZE_Y * 0.5f;
constexpr int	MAX_NUM_PLAYERS		= 2;
constexpr int	MAX_PLAYER_LIVES	= 4;
constexpr float RESTART_DELAY		= 3.f;
constexpr float STARSHIP_TITLE_START_X = 42.5f;
constexpr float STARSHIP_TITLE_START_Y = 70.f;

constexpr int MAX_ASTEROID_COUNT		 = 50;
constexpr int NUM_STARTING_ASTEROIDS	 = 4;
constexpr int NUM_ASTEROID_VERTICIES	 = 16;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.f;
constexpr float ASTEROID_PHYSICS_RADIUS	 = 1.6f;
constexpr float ASTEROID_SPEED			 = 10.f;
constexpr int	ASTEROID_MAX_HEALTH		 = 3;

constexpr int	MAX_BULLET_COUNT		= 30;
constexpr float BULLET_COSMETIC_RADIUS	= 2.0f;
constexpr float BULLET_PHYSICS_RADIUS	= 0.5f;
constexpr float BULLET_SPEED			= 50.f;
constexpr int	BULLET_MAX_HEALTH		= 1;

constexpr float PLAYER_SHIP_PHYSICS_RADIUS	= 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float PLAYER_SHIP_ACCELERATION	= 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED		= 300.f;
constexpr int	NUMBER_PLAYER_SHIP_DEBRIS	= 30;
constexpr float PLAYER_SHIP_MAX_SPEED		= 40.f;

constexpr float TRIGGER_ACTIVATION_FRACTION	= 0.6f;

constexpr float SCREEN_SHAKE_ABBERATION				= 1.f / 2.f;
constexpr float MAX_SCREEN_SHAKE_DISPLACEMENT		= 1.0f;
constexpr float PLAYER_DEATH_SCREEN_SHAKE_INTENSITY = 1.f;

constexpr int	MAX_BEETLE_COUNT		= 20;
constexpr float BEETLE_SPEED			= 10.f;
constexpr float BEETLE_COSMETIC_RADIUS	= 2.5f;
constexpr float BEETLE_PHYSICS_RADIUS	= 1.25f;
constexpr int	BEETLE_MAX_HEALTH		= 2;

constexpr int	MAX_WASP_COUNT			= 20;
constexpr float WASP_ACCELERATION		= 50.f;
constexpr float WASP_COSMETIC_RADIUS	= 2.4f;
constexpr float WASP_PHYSICS_RADIUS		= .75f;
constexpr int	WASP_MAX_HEALTH			= 2;
constexpr float WASP_MAX_SPEED			= 35.f;

constexpr int	NUM_DEBRIS_VERTS		= 20;
constexpr float DEBRIS_DURATION_SECONDS	= 2.f;
constexpr float DEBRIS_ALPHA_ABBERATION = 1.f / DEBRIS_DURATION_SECONDS;
constexpr int	MAX_DEBRIS_COUNT		= 1000;
constexpr float MAX_DEBRIS_SPEED		= 15.f;
constexpr float DEBRIS_SIZE_PERCENTAGE	= 0.6f;



// Microsoft Key Codes
constexpr unsigned char KEYCODE_ESCAPE		= 0x1B;
constexpr unsigned char KEYCODE_SPACE		= 0x20;
constexpr unsigned char KEYCODE_LEFT		= 0x25;
constexpr unsigned char KEYCODE_UP			= 0x26;
constexpr unsigned char KEYCODE_RIGHT		= 0x27;
constexpr unsigned char KEYCODE_F1			= 0x70;
constexpr unsigned char KEYCODE_F8			= 0x77;

// Colors
const Rgba8 RGBA8_BLACK				( 0, 0, 0);
const Rgba8 RGBA8_RED				( 255, 0, 0 );
const Rgba8 RGBA8_GREEN				( 0, 255, 0 );
const Rgba8 RGBA8_BLUE				( 0, 0, 255 );

const Rgba8 RGBA8_YELLOW			( 255, 255, 0 );
const Rgba8 RGBA8_MAGENTA			( 255, 0, 255);
const Rgba8 RGBA8_CYAN				( 0, 255, 255);

const Rgba8 RGBA8_BALI_HAI			( 132, 156, 169 ); //Teal Grey
const Rgba8 RGBA8_DARK_GREY			( 50, 50, 50 );
const Rgba8 RGBA8_MEDIUM_DARK_GREY	( 100, 100, 100 );
const Rgba8 RGBA8_TRANSPARENT_RED	( 255, 0, 0, 0 );

const Rgba8 BEETLE_COLOR			( 0, 150, 0 );
const Rgba8 WASP_COLOR				( 200, 170, 0 );


void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness );
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8 color, float thickness );