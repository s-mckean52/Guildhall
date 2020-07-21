#pragma once
#include "Game/RaycastResult.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include <string>

class Game;
class World;
class GPUMesh;
class PlayerStart;
class Camera;
class EntityDef;
class Entity;

class Map
{
public:
	Map( Game* theGame, World* theWorld );
	virtual ~Map();

	virtual void			Update()			= 0;
	virtual void			Render() const		= 0;

	virtual RaycastResult	Raycast( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance )	= 0;

	virtual void SpawnPlayer( Camera* playerCamera );
	virtual void SpawnNewEntityOnMap( XmlElement const& element );
	virtual Entity* SpawnNewEntityOfType( std::string const& entityDefName, XmlElement const& element );
	virtual Entity* SpawnNewEntityOfType( EntityDef const& entityDef, XmlElement const& element );

	virtual void AddEntityToMap( Entity* entityToAdd );
	virtual void RemoveEntityFromMap( Entity* entityToRemove );

	virtual Entity* GetClosestEntityInForwardSector( Vec3 const& sectorStartPosition, float maxDistanceToCheck, Vec3 const& forwardDirNormalized, float aperatureDegrees );

protected:
	virtual void AddEntityToList( std::vector<Entity*>& listToAddTo, Entity* entityToAdd );
	virtual void RemoveEntityFromList( std::vector<Entity*>& listToRemoveFrom, Entity* entityToRemove );
	virtual void HandleEntityVEntityCollisions();
	virtual void HandleEntityCollisions( Entity* effectedEntity );
	virtual void HandleEntityVEntityCollision( Entity* effectedEntity, Entity* effectorEntity );
	virtual void HandlePortalVEntityCollisions();
	virtual void HandlePortalVEntityCollision( Entity* portalEntity, Entity* entity );

protected:
	Game*		m_game		= nullptr;
	World*		m_world		= nullptr;
	GPUMesh*	m_mapMesh	= nullptr;

	Vec2 m_playerStartPositionXY;
	float	m_playerStartYawDegrees		= 0.f;
	Entity* m_playerStartEntity		= nullptr;

	std::vector<Entity*> m_entities;
	std::vector<Entity*> m_actors;
	std::vector<Entity*> m_projectiles;
	std::vector<Entity*> m_portals;

};