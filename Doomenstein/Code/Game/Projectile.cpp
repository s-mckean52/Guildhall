#include "Game/Projectile.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"

//---------------------------------------------------------------------------------------------------------
Projectile::Projectile( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef )
	: Entity( theGame, theWorld, theMap, entityDef )
{
}


//---------------------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}