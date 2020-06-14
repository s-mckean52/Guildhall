#pragma once
#include "Game/Entity.hpp"

class Camera;

class PlayerStart : public Entity
{
public:
	PlayerStart( Game* theGame, World* theWorld, Map* theMap, Vec2 const& position, float yaw );
	~PlayerStart();

	void StartPlayer( Camera* playerCamera );
};