#include "Game/GameObject.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
GameObject::GameObject( GPUMesh* mesh, Transform* transform, Rgba8 const& tint )
{
	m_mesh = mesh;
	m_transform = transform;
	m_tint = tint;
}


//---------------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
}


//---------------------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	delete m_mesh;
	m_mesh = nullptr;
}


//---------------------------------------------------------------------------------------------------------
Vec3 GameObject::GetPosition3D() const
{
	return m_transform->GetPosition();
}


//---------------------------------------------------------------------------------------------------------
Vec2 GameObject::GetPosition2D() const
{
	Vec3 position3D = GetPosition3D();
	return Vec2(position3D.x, position3D.z);
}


//---------------------------------------------------------------------------------------------------------
void GameObject::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void GameObject::Render() const
{
	g_theRenderer->SetCullMode(CULL_MODE_BACK);
	g_theRenderer->SetDepthTest(COMPARE_FUNC_LEQUAL, true);

	g_theRenderer->BindSampler(nullptr);
	g_theRenderer->BindShader((Shader*)nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindNormalTexture(nullptr);
	g_theRenderer->SetModelUBO( m_transform->ToMatrix(), m_tint, m_specFactor, m_specPower );
 	g_theRenderer->DrawMesh( m_mesh );
}
