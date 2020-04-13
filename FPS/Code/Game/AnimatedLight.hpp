#pragma once
#include "Engine/Renderer/Light.hpp"


enum LightMovement
{
	LIGHT_MOVEMENT_FOLLOW,
	LIGHT_MOVEMENT_ANIMATED,
	LIGHT_MOVEMENT_STATIONARY,
};


struct AnimatedLight
{
public:
	LightMovement moveType = LIGHT_MOVEMENT_STATIONARY;
	Light light;
};