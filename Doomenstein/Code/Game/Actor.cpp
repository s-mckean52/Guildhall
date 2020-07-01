#include "Game/Actor.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef )
	: Entity( theGame, theWorld, theMap, entityDef )
{
}


//---------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
}
