#pragma once
#include "Game/Entity.hpp"

class EntityDef;

class Portal : public Entity
{
public:
	Portal( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef );
	~Portal();
};