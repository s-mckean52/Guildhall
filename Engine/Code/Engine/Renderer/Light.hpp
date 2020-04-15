#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"


struct Light
{
public:
	Vec3 position;
	float intensity = 0.f;
	
	Vec3 direction = Vec3::FORWARD;
	float cosInnerHalfAngle = -1.f;
	
	Vec3 color = Vec3::UNIT;
	float cosOutterHalfAngle = -1.f;
	
	Vec3 attenuation = Vec3( 0.f, 1.f, 0.f );
	float isDirectional = 0.f;
	
	Vec3 specAttenuation = Vec3( 0.f, 1.f, 0.f );
	float padding = 0.f;

public:
	Light() = default;
	Light(	Vec3 const& toPosition,
			Vec3 const& toDirection,
			Vec3 const& toAttenuation,
			Vec3 const& toSpecAttenuation,
			Rgba8 const& toColor,
			float toIntensity,
			float innerHalfAngleDegrees,
			float outterHalfAngleDegrees,
			float toIsDirectional
	);

public:
	static const Light POINT;
	static const Light DIRECTIONAL;
	static const Light SPOTLIGHT;
};