#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
void PhysicsMaterial::AddBounciness( float bouncinessToAdd )
{
	m_bounciness += bouncinessToAdd;
	ClampZeroToOne( m_bounciness );
}


//---------------------------------------------------------------------------------------------------------
void PhysicsMaterial::AddFriction( float frictionToAdd )
{
	m_friction += frictionToAdd;
	ClampZeroToOne( m_friction );
}
