#include "Game/GameObject.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
}


//---------------------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	m_rigidbody->Destroy();
	m_rigidbody = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void GameObject::Draw() const
{
	m_rigidbody->m_collider->DebugRender( g_theRenderer, Rgba8::BLUE, Rgba8::GRAY );
}

