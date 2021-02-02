#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/TextureCube.hpp"
#include "Game/GameCommon.hpp"


//---------------------------------------------------------------------------------------------------------
TextureCube::TextureCube( RenderContext* owner )
{
	m_owner = owner;
}


//------------------------------------------------------------------------
bool TextureCube::MakeFromImages( Texture const *src )
{
	int width = src[0].GetImageTexelSize().x;
	int mip_levels = 1;

	// Setup our cubemap desc
	D3D11_TEXTURE2D_DESC cube_desc;
	cube_desc.Width				= width;
	cube_desc.Height			= width;
	cube_desc.MipLevels			= mip_levels;
	cube_desc.ArraySize			= TEXCUBE_SIDE_COUNT; // 6
	cube_desc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	cube_desc.Usage				= D3D11_USAGE_IMMUTABLE;
	cube_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	cube_desc.CPUAccessFlags	= 0;
	cube_desc.MiscFlags			= D3D11_RESOURCE_MISC_TEXTURECUBE; // IMPORTANT - Has to be marked as used as a cubemap
	
	// multisampling
	cube_desc.SampleDesc.Count = 1;
	cube_desc.SampleDesc.Quality = 0;
	
	// create the texture array
	// we're creating from 6 existing textures
	D3D11_SUBRESOURCE_DATA data[TEXCUBE_SIDE_COUNT]; // one per side - image
	D3D11_SUBRESOURCE_DATA *pdata = nullptr;
	if( nullptr != src ) 
	{
	   pdata = data;
	   uint pitch = width * 4;
	
	   for( uint i = 0; i < TEXCUBE_SIDE_COUNT; ++i )
	   {
		   unsigned char* imageData = src[i].GetPixelData();
		   data[i].pSysMem = imageData;
		   data[i].SysMemPitch = pitch;
		   data[i].SysMemSlicePitch = 0;
	   }
	}
	
	// create my resource
	g_theRenderer->m_device->CreateTexture2D( &cube_desc, pdata, &m_handle );
	
	// create my view
	CreateViews();
	
	return true;
}


//------------------------------------------------------------------------
bool TextureCube::MakeFromImage( Texture const &src )
{
	uint width = src.GetImageTexelSize().x / 4;
	uint height = src.GetImageTexelSize().y / 3;
	int mip_levels = 1;

	// Setup our cubemap desc
	D3D11_TEXTURE2D_DESC cube_desc;
	cube_desc.Width				= width;
	cube_desc.Height			= height;
	cube_desc.MipLevels			= mip_levels;
	cube_desc.ArraySize			= TEXCUBE_SIDE_COUNT; // 6
	cube_desc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	cube_desc.Usage				= D3D11_USAGE_IMMUTABLE;
	cube_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	cube_desc.CPUAccessFlags	= 0;
	cube_desc.MiscFlags			= D3D11_RESOURCE_MISC_TEXTURECUBE; // IMPORTANT - Has to be marked as used as a cubemap

   // multisampling
   cube_desc.SampleDesc.Count = 1;
   cube_desc.SampleDesc.Quality = 0;

   // create the texture array
   // we're creating from 6 existing textures
   D3D11_SUBRESOURCE_DATA data[TEXCUBE_SIDE_COUNT];
   D3D11_SUBRESOURCE_DATA *pdata = nullptr;
   if( true ) 
   {
      pdata = data;
      uint pitch = width * 4; // width of a single part in bytes
	  unsigned char* start = src.GetPixelData();

      uint total_pitch = 4 * pitch; // total with of entire image in bytes
      uint row = width * total_pitch; // how far to get to the next row

	  //+x
	  //-x
	  //+y
	  //-y
	  //+z
	  //-z
      uint offsets[] = {
         (1 * row) + (2 * pitch),    // right
         (1 * row) + (0 * pitch),    // left
         (0 * row) + (1 * pitch),    // top
         (2 * row) + (1 * pitch),    // bottom
         (1 * row) + (1 * pitch),    // front
         (1 * row) + (3 * pitch),    // back
      };
      
      for (uint i = 0; i < TEXCUBE_SIDE_COUNT; ++i) {
         data[i].pSysMem = start + offsets[i];
         data[i].SysMemPitch = total_pitch;
         data[i].SysMemSlicePitch = 0;
      }
   }

	// create my resource
	g_theRenderer->m_device->CreateTexture2D( &cube_desc, pdata, &m_handle );

   CreateViews();
   return true;
}

//------------------------------------------------------------------------
void TextureCube::CreateViews()
{
   CD3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
   view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
   view_desc.TextureCube.MostDetailedMip = 0;
   view_desc.TextureCube.MipLevels = 1;
   
	ID3D11Device* device = m_owner->m_device;
	ID3D11ShaderResourceView* srv = nullptr;
	device->CreateShaderResourceView( m_handle, &view_desc, &srv );

	if( srv != nullptr )
	{
		m_shaderResourceView = new TextureView();
		m_shaderResourceView->m_srv = srv;
	}
}