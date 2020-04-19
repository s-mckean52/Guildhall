#include "Game/Lamp.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Lamp::Lamp(Vec3 const& position)
{
	float outterHalfAngle = 30.f;
	m_transform = new Transform( position );
	m_light.direction = -Vec3::UP;
	m_light.position = position;
	m_light.position.y = 2.f;
	m_light.intensity = 3.f;
	m_light.cosInnerHalfAngle = CosDegrees(3.f);
	m_light.cosOutterHalfAngle = CosDegrees(outterHalfAngle);
	m_collisionDiscRadius = SinDegrees( outterHalfAngle ) * m_light.position.y;
}


//---------------------------------------------------------------------------------------------------------
void Lamp::EnableLight( unsigned int indexToUse )
{
	g_theRenderer->EnableLight( indexToUse, m_light );
}


//---------------------------------------------------------------------------------------------------------
bool Lamp::DiscIsInLight(Vec2 const& discCenter2D, float discRadius)
{
	return DoDiscsOverlap( discCenter2D, discRadius, GetPosition2D(), m_collisionDiscRadius );
}

