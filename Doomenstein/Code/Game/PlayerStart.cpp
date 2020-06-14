#include "Game/PlayerStart.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"

//---------------------------------------------------------------------------------------------------------
PlayerStart::PlayerStart( Game* theGame, World* theWorld, Map* theMap, Vec2 const& position, float yaw )
	: Entity( theGame, theWorld, theMap )
{
	m_position = position;
	m_yaw = yaw;
}


//---------------------------------------------------------------------------------------------------------
PlayerStart::~PlayerStart()
{
}


//---------------------------------------------------------------------------------------------------------
void PlayerStart::StartPlayer( Camera* playerCamera )
{
	playerCamera->SetPosition( Vec3( m_position, PLAYER_HEIGHT ) );
	playerCamera->SetPitchYawRollRotationDegrees( 0.f, m_yaw, 0.f );
}
