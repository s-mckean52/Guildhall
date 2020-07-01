#pragma once
#include "Game/Entity.hpp"

class EntityDef;

class Actor : public Entity
{
public:
	Actor( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef );
	~Actor();
};