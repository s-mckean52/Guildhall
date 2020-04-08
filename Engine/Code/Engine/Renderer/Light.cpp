#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/Vec4.hpp"


//---------------------------------------------------------------------------------------------------------
Light::Light( Vec3 const& toPosition, Rgba8 const& toColor, float toIntensity, Vec3 const& toAttenuation, Vec3 const& toSpecAttenuation )
{
	Vec4 colorAsFloats = toColor.GetValuesAsFractions();
	color.x = colorAsFloats.x;
	color.y = colorAsFloats.y;
	color.z = colorAsFloats.z;
	position = toPosition;
	intensity = toIntensity;
	attenuation = toAttenuation;
	specAttenuation = toSpecAttenuation;
}
