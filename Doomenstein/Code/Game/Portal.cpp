#include "Game/Portal.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"

//---------------------------------------------------------------------------------------------------------
Portal::Portal( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef )
	: Entity( theGame, theWorld, theMap, entityDef )
{
}


//---------------------------------------------------------------------------------------------------------
Portal::~Portal()
{
}