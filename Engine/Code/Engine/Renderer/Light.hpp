#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"


struct Light
{
public:
	Vec3 position;
	float intensity = 0.f;

	Vec3 color = Vec3::UNIT;
	float padding_00 = 0.f;

	Vec3 attenuation = Vec3( 0.f, 1.f, 0.f );
	float padding_01 = 0.f;

	Vec3 specAttenuation = Vec3( 0.f, 1.f, 0.f );
	float padding_02 = 0.f;

public:
	Light() = default;
	Light( Vec3 const& toPosition, Rgba8 const& toColor, float toIntensity, Vec3 const& toAttenuation, Vec3 const& toSpecAttenuation );
};