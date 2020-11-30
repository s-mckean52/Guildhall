#include "Engine/Core/DebugRender.hpp"
#include "Game/Game.hpp"
#include "Game/Projectile.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"

//---------------------------------------------------------------------------------------------------------
Projectile::Projectile( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element )
	: Entity( theGame, theWorld, theMap, entityDef, element )
{
	m_mass					= 1.f;
	m_canBePushedByWalls	= true;
	m_canBePushedByEntities = false;
	m_canPushEntities		= false;
}


//---------------------------------------------------------------------------------------------------------
Projectile::Projectile( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef )
	: Entity( theGame, theWorld, theMap, entityDef )
{
	m_mass					= 1.f;
	m_canBePushedByWalls	= true;
	m_canBePushedByEntities = false;
	m_canPushEntities		= false;
}


//---------------------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Update()
{
	float deltaSeconds = m_theGame->GetDeltaSeconds();
	m_position += Vec3( m_forwardDirection * GetSpeed() * deltaSeconds, 0.f );
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Render() const
{
	DebugAddWorldWireSphere( m_position, GetPhysicsRadius(), Rgba8::ORANGE, 0.f );
}
