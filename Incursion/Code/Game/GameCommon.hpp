#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Game/Tile.hpp"
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

extern App* g_theApp;
extern Game* g_theGame;
extern RenderContext* g_theRenderer;
extern RandomNumberGenerator* g_RNG;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern BitmapFont* g_testFont;

extern bool g_isDebugCamera;
extern bool g_isDebugDraw;
extern bool g_isNoClip;


//---------------------------------------------------------------------------------------------------------
// Global Game-specific Constants
constexpr float MAX_FRAME_TIME			= .1f;
constexpr float CAMERA_SIZE_X			= 16.f;
constexpr float CAMERA_SIZE_Y			= 9.f;
constexpr float CLIENT_ASPECT			= CAMERA_SIZE_X / CAMERA_SIZE_Y;
constexpr float HALF_SCREEN_X			= CAMERA_SIZE_X * 0.5f;
constexpr float HALF_SCREEN_Y			= CAMERA_SIZE_Y * 0.5f;
constexpr int	MAX_NUM_PLAYERS			= 1;
constexpr int	EXTRA_PLAYER_LIVES		= 3;
constexpr float RESTART_DELAY			= 3.f;
constexpr int	NUM_MAPS				= 3;
constexpr float TILE_SIZE				= 1.f;
constexpr float DEBUG_THICKNESS			= 0.02f;
constexpr float RAYCAST_STEP_PER_UNIT	= 100.f;
constexpr float RAYCAST_STEP_DISTANCE	= 1.f / RAYCAST_STEP_PER_UNIT;

constexpr float TRIGGER_ACTIVATION_FRACTION	= 0.6f;

constexpr float SCREEN_SHAKE_ABBERATION				= 1.f / 2.f;
constexpr float MAX_SCREEN_SHAKE_DISPLACEMENT		= 1.0f;
constexpr float PLAYER_DEATH_SCREEN_SHAKE_INTENSITY = 1.f;

constexpr float DELAY_AFTER_DEATH_SECONDS			= 2.f;
constexpr float DELAY_BEFORE_RESPAWN_SECONDS		= 0.5f;

constexpr float DELAY_BEFORE_VICTORY_SECONDS		= 0.5f;

constexpr float HEALTH_BAR_HEIGHT					= 0.1f;
constexpr float HEALTH_REGEN_TIME_SECONDS			= 3.f;

//---------------------------------------------------------------------------------------------------------
// Player Constants
constexpr float	PLAYER_ACCELERATION					= 20.f;
constexpr float PLAYER_MAX_SPEED					= 1.f;
constexpr float PLAYER_FRICTION_PER_SECOND			= 11.f;
constexpr float PLAYER_PHYSICS_RADIUS				= 0.3f;
constexpr float PLAYER_COSMETIC_RADIUS				= 0.4f;
constexpr float PLAYER_ANGULAR_VELOCITY				= 90.f;
constexpr float PLAYER_START_ORIENTATION			= 90.f;
constexpr float PLAYER_TURRET_ANGULAR_VELOCITY		= 180.f;
constexpr int	PLAYER_HEALTH						= 3;
constexpr float PLAYER_EXPLOSION_DURATION_SECONDS	= 2.5f;

//---------------------------------------------------------------------------------------------------------
// NPC Tank Constants
constexpr float NPC_TANK_PHYSICS_RADIUS				= 0.3f;
constexpr float NPC_TANK_COSMETIC_RADIUS			= 0.4f;
constexpr float NPC_TANK_ANGULAR_VELOCITY			= 30.f;
constexpr float NPC_TANK_VIEW_RANGE					= 10.f;
constexpr float NPC_TANK_DRIVE_APERATURE_DEGREES	= 90.f;
constexpr float NPC_TANK_SHOOT_INTERVAL				= 1.7f;
constexpr float NPC_TANK_SHOOT_APERATURE_DEGREES	= 10.f;
constexpr float NPC_TANK_SPEED						= 0.7f;
constexpr float NPC_TANK_NEW_ORIENTATION_INTERVAL	= 3.f;
constexpr int	NPC_TANK_HEALTH						= 2;
constexpr float NPC_TANK_EXPLOSION_DURATION_SECONDS = 1.f;

//---------------------------------------------------------------------------------------------------------
// NPC Turret Constants
constexpr float NPC_TURRET_PHYSICS_RADIUS			= 0.3f;
constexpr float NPC_TURRET_COSMETIC_RADIUS			= 0.4f;
constexpr float NPC_TURRET_VIEW_RANGE				= 10.f;
constexpr float NPC_TURRET_ANGULAR_VELOCITY			= 20.f;
constexpr float NPC_TURRET_LASER_THICKNESS			= 0.015f;
constexpr float NPC_TURRET_SHOOT_INTERVAL			= 1.3f;
constexpr float NPC_TURRET_SHOOT_APERATURE_DEGREES	= 10.f;
constexpr int	NPC_TURRET_HEALTH					= 2;
constexpr float NPC_TURRET_SWIVEL_APERATURE_DEGREES	= 90.f;
constexpr float NPC_TURRET_EXPLOSION_DURATION_SECONDS = 1.f;

//---------------------------------------------------------------------------------------------------------
// Bullet Constants
constexpr float BULLET_VELOCITY						= 5.f;
constexpr float BULLET_PHYSICS_RADIUS				= 0.05f;
constexpr float BULLET_COSMETIC_RADIUS				= 0.1f;
constexpr float BULLET_EXPLOSION_DURATION			= 0.5f;
constexpr float BULLET_EXPLOSION_RADIUS				= 0.1f;

//---------------------------------------------------------------------------------------------------------
// Boulder Constants
constexpr float BOULDER_RADIUS						= 0.35f;

//---------------------------------------------------------------------------------------------------------
// Tile Constants
constexpr float MUD_SLOW_FRACTION					= 0.5f;


//---------------------------------------------------------------------------------------------------------
//
//	Map Constants
//
//---------------------------------------------------------------------------------------------------------
//	Map 1
constexpr int		MAP_1_SIZE_X					= 20;
constexpr int		MAP_1_SIZE_Y					= 30;
constexpr TileType	MAP_1_DEFAULT_TILE_TYPE			= TILE_TYPE_GRASS;
constexpr TileType	MAP_1_EDGE_TILE_TYPE			= TILE_TYPE_STONE;
constexpr TileType	MAP_1_START_TILE_TYPE			= TILE_TYPE_GRASS;
constexpr TileType	MAP_1_END_TILE_TYPE				= TILE_TYPE_GRASS;
constexpr int		MAP_1_NUM_ENTITIES_TO_SPAWN		= 16;

//		Worm 1
constexpr TileType	MAP_1_WORM_1_TYPE				= TILE_TYPE_STONE;
constexpr int		MAP_1_WORM_1_NUM				= 25;
constexpr int		MAP_1_WORM_1_LENGTH				= 8;


//---------------------------------------------------------------------------------------------------------
//	Map 2
constexpr int		MAP_2_SIZE_X					= 30;
constexpr int		MAP_2_SIZE_Y					= 45;
constexpr TileType	MAP_2_DEFAULT_TILE_TYPE			= TILE_TYPE_WOOD;
constexpr TileType	MAP_2_EDGE_TILE_TYPE			= TILE_TYPE_SHINGLES;
constexpr TileType	MAP_2_START_TILE_TYPE			= TILE_TYPE_WOOD;
constexpr TileType	MAP_2_END_TILE_TYPE				= TILE_TYPE_WOOD;
constexpr int		MAP_2_NUM_ENTITIES_TO_SPAWN		= 24;

//		Worm 1
constexpr TileType	MAP_2_WORM_1_TYPE				= TILE_TYPE_SHINGLES;
constexpr int		MAP_2_WORM_1_NUM				= 35;
constexpr int		MAP_2_WORM_1_LENGTH				= 16;
//		Worm 2
constexpr TileType	MAP_2_WORM_2_TYPE				= TILE_TYPE_MUD;
constexpr int		MAP_2_WORM_2_NUM				= 17;
constexpr int		MAP_2_WORM_2_LENGTH				= 12;

//---------------------------------------------------------------------------------------------------------
//	Map 3
constexpr int		MAP_3_SIZE_X					= 60;
constexpr int		MAP_3_SIZE_Y					= 30;
constexpr TileType	MAP_3_DEFAULT_TILE_TYPE			= TILE_TYPE_SAND;
constexpr TileType	MAP_3_EDGE_TILE_TYPE			= TILE_TYPE_STONE_WALL;
constexpr TileType	MAP_3_START_TILE_TYPE			= TILE_TYPE_SAND;
constexpr TileType	MAP_3_END_TILE_TYPE				= TILE_TYPE_SAND;
constexpr int		MAP_3_NUM_ENTITIES_TO_SPAWN		= 36;
						
//		Worm 1			
constexpr TileType	MAP_3_WORM_1_TYPE				= TILE_TYPE_STONE_WALL;
constexpr int		MAP_3_WORM_1_NUM				= 30;
constexpr int		MAP_3_WORM_1_LENGTH				= 17;
//		Worm 2			
constexpr TileType	MAP_3_WORM_2_TYPE				= TILE_TYPE_MUD;
constexpr int		MAP_3_WORM_2_NUM				= 20;
constexpr int		MAP_3_WORM_2_LENGTH				= 12;
//		Worm 3			
constexpr TileType	MAP_3_WORM_3_TYPE				= TILE_TYPE_GRASS;
constexpr int		MAP_3_WORM_3_NUM				= 15;
constexpr int		MAP_3_WORM_3_LENGTH				= 12;

//---------------------------------------------------------------------------------------------------------


// Game Specific Colors
const Rgba8 RGBA8_BALI_HAI			( 132, 156, 169 ); //Teal Grey
const Rgba8 RGBA8_DARK_GREY			( 50, 50, 50 );
const Rgba8 RGBA8_MEDIUM_DARK_GREY	( 100, 100, 100 );
const Rgba8 RGBA8_TRANSPARENT_RED	( 255, 0, 0, 0 );

const Rgba8 RGBA8_DARK_GREEN		( 0, 150, 0 );
const Rgba8 RGBA8_DARK_YELLOW		( 200, 170, 0 );


// Common Game Functions
void DrawLineBetweenPoints	( const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness );
void DrawCircleAtPoint		( const Vec2& position, float radius, const Rgba8 color, float thickness );