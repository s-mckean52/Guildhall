#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/Vec4.hpp"


//---------------------------------------------------------------------------------------------------------
Light::Light( Vec3 const& toPosition, Rgba8 const& toColor, float toIntensity )
{
	Vec4 colorAsFloats = toColor.GetValuesAsFractions();
	color.x = colorAsFloats.x;
	color.y = colorAsFloats.y;
	color.z = colorAsFloats.z;
	position = toPosition;
	intensity = toIntensity;
}
