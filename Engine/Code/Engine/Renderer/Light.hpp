#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"


struct Light
{
public:
	Vec3 position;
	float intensity = 0.f;
	Vec3 color = Vec3::UNIT;
	float padding = 0.f;

public:
	Light() = default;
	Light( Vec3 const& toPosition, Rgba8 const& toColor, float toIntensity );
};