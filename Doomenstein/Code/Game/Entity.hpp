#pragma once
#include "Game/EntityDef.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>

class Game;
class World;
class Map;

struct EntityData
{
	bool	m_isPossessed			= false;
	bool	m_canBePushedByWalls	= true;
	bool	m_canBePushedByEntities = true;
	bool	m_canPushEntities		= true;
	float	m_mass					= 1.f;
	bool	m_isDead				= true;
	int		m_currentHealth			= 0;

	Vec3		m_position;
	Vec2		m_forwardDirection;
	float		m_yaw				= 0.f;
	char		m_actionState[25]	= "Walk";
	char		m_entityDefName[50] = "";
};

struct EntitySpawnData
{
	bool m_isUsed = false;
	char m_entityDefName[50] = "";
	EntityData m_data;
};


//---------------------------------------------------------------------------------------------------------
class Entity
{
public:
	static int s_entityID;

	explicit Entity( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef );
	explicit Entity( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element );
	virtual ~Entity();

	//---------------------------------------------------------------------------------------------------------
	virtual void Update();
	virtual void Render() const;
	virtual void RenderHealthBar() const;
	virtual void DebugRender() const;
	virtual void SetValuesFromXML( XmlElement const& element );
	virtual void SetValuesFromEntityData( EntityData const& entityData );

	//---------------------------------------------------------------------------------------------------------
	std::string GetEntityName() const;
	EntityType	GetEntityType() const;
	Vec3 const&	GetPosition() const			{ return m_position; }
	float		GetYaw() const				{ return m_yaw; }
	bool		IsPossessed() const			{ return m_isPossessed; }
	bool		IsPushedByWalls() const		{ return m_canBePushedByWalls; }
	bool		IsPushedByEntities() const	{ return m_canBePushedByEntities; }
	bool		CanPushEntities() const		{ return m_canPushEntities; }
	float		GetMass() const				{ return m_mass; }
	int			GetHealth() const			{ return m_currentHealth; }
	bool		GetIsDead() const			{ return m_isDead; }
	float		GetHeight() const;
	float		GetEyeHeight() const;
	float		GetSpeed() const;
	float		GetPhysicsRadius() const;
	EntityData	GetEntityData() const;

	void UpdateAnimDirection();
	void CheckAndUpdateSpriteDirection( Vec2 const& directionToCompare, std::string const& directionName, Vec2 const& direction );

	//---------------------------------------------------------------------------------------------------------
	void SetPosition( Vec3 const& position );
	void SetForward( Vec2 const& forward );
	void SetYaw( float yaw );
	void AddYaw( float yawToAdd );
	void Translate( Vec3 const& translation );
	void SetIsPossessed( bool isPossesed );
	void SetMap( Map* map );
	void TakeDamage( int damageToTake );

	//---------------------------------------------------------------------------------------------------------
	void Shoot();

protected:
	Game*	m_theGame	= nullptr;
	World*	m_theWorld	= nullptr;
	Map*	m_theMap	= nullptr;
	uint	m_id		= 0;

	bool m_isPossessed = false;

	Vec3 m_bottomLeft;
	Vec3 m_bottomRight;
	Vec3 m_topRight;
	Vec3 m_topLeft;

	bool	m_canBePushedByWalls	= true;
	bool	m_canBePushedByEntities = true;
	bool	m_canPushEntities		= true;
	float	m_mass = 1.f;

	EntityDef const& m_entityDef;

	bool m_isDead = false;
	int m_currentHealth = 10;

	Vec3		m_position;
	Vec2		m_forwardDirection;
	float		m_yaw				= 0.f;
	std::string	m_actionState		= "Walk";
	std::string m_currentSpriteDirection = "front";
};