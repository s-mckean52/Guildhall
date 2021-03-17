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

	delete m_depthStencilView;
	m_depthStencilView = nullptr;

// 	if( m_pixelData != nullptr )
// 	{
// 		delete[] m_pixelData;
// 		m_pixelData = nullptr;
// 	}

	DX_SAFE_RELEASE( m_handle );
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
void Texture::SetPixelData( int size, unsigned char* pixelData )
{
	m_pixelData = new unsigned char[size];
	memcpy( m_pixelData, pixelData, size );
}


//---------------------------------------------------------------------------------------------------------
bool Texture::AreViewsMatching( Texture* textureToCompare ) const
{
	if( ( IsRenderTarget() && textureToCompare->IsRenderTarget() )		&&
		( IsShaderResourse() && textureToCompare->IsShaderResourse() )	&&
		( IsDepthStencil() && textureToCompare->IsDepthStencil() )			)
	{
		return true;
	}
	return false;
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

	if( m_depthStencilView != nullptr )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
		sr_desc.Format						= DXGI_FORMAT_R32_FLOAT;
		sr_desc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
		sr_desc.Texture2D.MostDetailedMip	= 0;
		sr_desc.Texture2D.MipLevels			= 1;

		device->CreateShaderResourceView( m_handle, &sr_desc, &srv );
	}
	else
	{
		device->CreateShaderResourceView( m_handle, nullptr, &srv );
	}

	if( srv != nullptr )
	{
		m_shaderResourceView = new TextureView();
		m_shaderResourceView->m_srv = srv;
	}

	return m_shaderResourceView;
}


//---------------------------------------------------------------------------------------------------------
TextureView* Texture::GetOrCreateDepthStencilView()
{
	if( m_depthStencilView != nullptr )
	{
		return m_depthStencilView;
	}

	ID3D11Device* device = m_owner->m_device;
	ID3D11DepthStencilView* dsv = nullptr;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = 0;
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView( m_handle, &dsv_desc, &dsv );

	if( dsv != nullptr )
	{
		m_depthStencilView = new TextureView();
		m_depthStencilView->m_dsv = dsv;
	}

	return m_depthStencilView;
}
