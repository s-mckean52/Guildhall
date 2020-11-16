#pragma once
#include "Game/Entity.hpp"

class EntityDef;
class Map;

class Portal : public Entity
{
public:
	Portal( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef );
	Portal( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element );
	~Portal();

	virtual void SetValuesFromXML( XmlElement const& element )	override;

	void UsePortal( Entity* entityUsingPortal );

private:
	Vec2		m_destinationPos;
	float		m_destinationYawOffset	= 0.f;
	std::string	m_destinationMapName	= "";
};