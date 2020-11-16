#pragma once
#include "Game/Entity.hpp"

class EntityDef;

class Projectile : public Entity
{
public:
	Projectile( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef );
	Projectile( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element );
	~Projectile();
};