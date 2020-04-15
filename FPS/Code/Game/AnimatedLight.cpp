#include "Game/AnimatedLight.hpp"


//---------------------------------------------------------------------------------------------------------
std::string AnimatedLight::GetLightTypeAsString() const
{
	switch( lightType )
	{
	case LIGHT_TYPE_POINT:
		return "POINT";
	case LIGHT_TYPE_DIRECTIONAL:
		return "DIRECTIONAL";
	case LIGHT_TYPE_SPOTLIGHT:
		return "SPOT LIGHT";
	default:
		return "ERROR";
	}
}


//---------------------------------------------------------------------------------------------------------
void AnimatedLight::SetLightType( LightType typeToSetTo )
{
	lightType = typeToSetTo;
	Light lightToCopy;

	switch( typeToSetTo )
	{
	case LIGHT_TYPE_POINT:
		lightToCopy = Light::POINT;
		break;
	case LIGHT_TYPE_DIRECTIONAL:
		lightToCopy = Light::DIRECTIONAL;
		break;
	case LIGHT_TYPE_SPOTLIGHT:
		lightToCopy = Light::SPOTLIGHT;
		break;
	default:
		break;
	}

	light.isDirectional = lightToCopy.isDirectional;
	light.cosInnerHalfAngle = lightToCopy.cosInnerHalfAngle;
	light.cosOutterHalfAngle = lightToCopy.cosOutterHalfAngle;
}
