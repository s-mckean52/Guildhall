#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView.hpp"

//---------------------------------------------------------------------------------------------------------
Texture::~Texture()
{
	m_owner = nullptr;

	delete m_renderTargetView;
	m_renderTargetView = nullptr;

	DX_SAFE_RELEASE( m_handle );
}


//---------------------------------------------------------------------------------------------------------
Texture::Texture( const char* imageFilePath, int textureID, IntVec2 imageTexelSize, int numComponents )
	: m_imageFilePath( imageFilePath )
	, m_textureID( textureID )
	, m_imageTexelSize( imageTexelSize )
	, m_numComponents( numComponents )
{
}


//---------------------------------------------------------------------------------------------------------
Texture::Texture( RenderContext* renderContext, ID3D11Texture2D* handle )
	: m_owner( renderContext )
	, m_handle( handle )
{
	D3D11_TEXTURE2D_DESC desc;
	handle->GetDesc( &desc );
	m_imageTexelSize = IntVec2( desc.Width, desc.Height );
}


//---------------------------------------------------------------------------------------------------------
float Texture::GetAspect() const
{
	return static_cast<float>( m_imageTexelSize.x ) / static_cast<float>( m_imageTexelSize.y );
}


//---------------------------------------------------------------------------------------------------------
TextureView* Texture::GetRenderTargetView()
{
	if( m_renderTargetView )
	{
		return m_renderTargetView;
	}

	ID3D11Device* device = m_owner->m_device;
	ID3D11RenderTargetView* rtv = nullptr;
	device->CreateRenderTargetView( m_handle, nullptr, &rtv );

	if( rtv != nullptr )
	{
		m_renderTargetView = new TextureView();
		m_renderTargetView->m_rtv = rtv;
	}

	return m_renderTargetView;
}

