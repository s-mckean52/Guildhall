#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/EngineCommon.hpp"

//---------------------------------------------------------------------------------------------------------
Light::Light(	Vec3 const& toPosition,
				Vec3 const& toDirection,
				Vec3 const& toAttenuation,
				Vec3 const& toSpecAttenuation,
				Rgba8 const& toColor,
				float toIntensity,
				float innerHalfAngleDegrees,
				float outterHalfAngleDegrees,
				float toIsDirectional )
{
	position = toPosition;
	direction = toDirection;
	attenuation = toAttenuation;
	specAttenuation = toSpecAttenuation;

	Vec4 colorAsFloats = toColor.GetValuesAsFractions();
	color.x = colorAsFloats.x;
	color.y = colorAsFloats.y;
	color.z = colorAsFloats.z;
	
	intensity = toIntensity;
	cosInnerHalfAngle = CosDegrees( innerHalfAngleDegrees );
	cosOutterHalfAngle = CosDegrees( outterHalfAngleDegrees );
	isDirectional = toIsDirectional;
}


//---------------------------------------------------------------------------------------------------------
STATIC const Light Light::POINT = Light(
	/*position*/				Vec3::ZERO,
	/*Direction*/				Vec3::FORWARD,
	/*Attenuation*/				Vec3( 0.f, 1.f, 0.f ),
	/*Specular Attenuation*/	Vec3( 0.f, 1.f, 0.f ),
	/*Color*/					Rgba8::WHITE,
	/*Intensity*/				0.f,
	/*Inner Half Angle*/		180.f,
	/*Outter Half Angle*/		180.f,
	/*Is Directional*/			0.f
);


STATIC const Light Light::DIRECTIONAL = Light(
	/*position*/				Vec3::ZERO,
	/*Direction*/				Vec3::FORWARD,
	/*Attenuation*/				Vec3( 0.f, 1.f, 0.f ),
	/*Specular Attenuation*/	Vec3( 0.f, 1.f, 0.f ),
	/*Color*/					Rgba8::WHITE,
	/*Intensity*/				0.f,
	/*Inner Half Angle*/		180.f,
	/*Outter Half Angle*/		180.f,
	/*Is Directional*/			1.f
);


STATIC const Light Light::SPOTLIGHT = Light(
	/*position*/				Vec3::ZERO,
	/*Direction*/				Vec3::FORWARD,
	/*Attenuation*/				Vec3( 0.f, 1.f, 0.f ),
	/*Specular Attenuation*/	Vec3( 0.f, 1.f, 0.f ),
	/*Color*/					Rgba8::WHITE,
	/*Intensity*/				0.f,
	/*Inner Half Angle*/		15.f,
	/*Outter Half Angle*/		30.f,
	/*Is Directional*/			0.f
);