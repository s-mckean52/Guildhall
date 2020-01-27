#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------------------------
SwapChain::SwapChain( RenderContext* owner, IDXGISwapChain* handle )
	: m_owner( owner )
	, m_handle( handle )
{
}


//---------------------------------------------------------------------------------------------------------
SwapChain::~SwapChain()
{
	m_owner = nullptr;

	delete m_backbuffer;
	m_backbuffer = nullptr;
	
	DX_SAFE_RELEASE( m_handle );
}


//---------------------------------------------------------------------------------------------------------
void SwapChain::Present( int vsync )
{
	m_handle->Present( vsync, 0 );
}


//---------------------------------------------------------------------------------------------------------
Texture* SwapChain::GetBackBuffer()
{
	if( m_backbuffer != nullptr )
	{
		return m_backbuffer;
	}

	ID3D11Texture2D* texHandle = nullptr;
	m_handle->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( void** )&texHandle );
	ASSERT_OR_DIE( m_handle != nullptr, "Failed to get backbuffer" );

	m_backbuffer = new Texture( m_owner, texHandle );

	return m_backbuffer;
}
