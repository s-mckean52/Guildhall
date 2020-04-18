#pragma once
#include "Engine/Renderer/Light.hpp"
#include <string>

enum LightMovement
{
	LIGHT_MOVEMENT_FOLLOW,
	LIGHT_MOVEMENT_ANIMATED,
	LIGHT_MOVEMENT_STATIONARY,
};


enum LightType
{
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_SPOTLIGHT,

	NUM_LIGHT_TYPES,
};


struct AnimatedLight
{
public:
	LightType lightType = LIGHT_TYPE_POINT;
	LightMovement moveType = LIGHT_MOVEMENT_STATIONARY;
	Light light;

public:
	std::string GetLightTypeAsString() const;
	void SetLightType( LightType typeToSetTo );
};