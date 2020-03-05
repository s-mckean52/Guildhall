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

	delete m_shaderResourceView;
	m_shaderResourceView = nullptr;

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
Texture::Texture( const char* filePath, RenderContext* owner, ID3D11Texture2D* handle )
	: Texture( owner, handle )
{
	m_imageFilePath = filePath;
}


//---------------------------------------------------------------------------------------------------------
float Texture::GetAspect() const
{
	return static_cast<float>( m_imageTexelSize.x ) / static_cast<float>( m_imageTexelSize.y );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Texture::GetSize() const
{
	Vec2 size;

	size.x = static_cast<float>( m_imageTexelSize.x );
	size.y = static_cast<float>( m_imageTexelSize.y );

	return size;
}


//---------------------------------------------------------------------------------------------------------
TextureView* Texture::CreateOrGetRenderTargetView()
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


//---------------------------------------------------------------------------------------------------------
TextureView* Texture::GetOrCreateShaderResourceView()
{
	if( m_shaderResourceView != nullptr )
	{
		return m_shaderResourceView;
	}

	ID3D11Device* device = m_owner->m_device;
	ID3D11ShaderResourceView* srv = nullptr;
	device->CreateShaderResourceView( m_handle, nullptr, &srv );

	if( srv != nullptr )
	{
		m_shaderResourceView = new TextureView();
		m_shaderResourceView->m_srv = srv;
	}

	return m_shaderResourceView;
}