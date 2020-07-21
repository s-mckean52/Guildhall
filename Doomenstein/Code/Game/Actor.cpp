#include "Game/Actor.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element )
	: Entity( theGame, theWorld, theMap, entityDef, element )
{
}


//---------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
}
