#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
Sampler::Sampler( RenderContext* context, SamplerType type, TextureAddressMode textureMode )
{
	m_handle = nullptr;
	m_owner = context;

	ID3D11Device* device = context->m_device;
	
	D3D11_SAMPLER_DESC desc;

	if( type == SAMPLER_POINT )
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	else if( type == SAMPLER_BILINEAR )
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	}

	desc.AddressU = ToD3D11TextureAddress( textureMode );
	desc.AddressV = ToD3D11TextureAddress( textureMode );
	desc.AddressW = ToD3D11TextureAddress( textureMode );

	desc.MipLODBias			= 0.0f;
	desc.MaxAnisotropy		= 0;
	desc.ComparisonFunc		= D3D11_COMPARISON_NEVER;
	
	desc.BorderColor[ 0 ] = 0.0f;
	desc.BorderColor[ 1 ] = 0.0f;
	desc.BorderColor[ 2 ] = 0.0f;
	desc.BorderColor[ 3 ] = 0.0f;

	desc.MinLOD = 0;
	desc.MaxLOD = 0;

	device->CreateSamplerState( &desc, &m_handle );
}


//---------------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{
	m_owner = nullptr;
	DX_SAFE_RELEASE( m_handle );
}


//---------------------------------------------------------------------------------------------------------
bool Sampler::IsMatching( SamplerType type, TextureAddressMode textureMode )
{
	D3D11_SAMPLER_DESC desc;
	m_handle->GetDesc( &desc );

	if( type == SAMPLER_POINT )
	{
		if( desc.Filter != D3D11_FILTER_MIN_MAG_MIP_POINT )
			return false;
	}
	else if( type == SAMPLER_BILINEAR )
	{
		if( desc.Filter != D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT )
			return false;
	}

	if( desc.AddressU != ToD3D11TextureAddress( textureMode ) ) 
		return false;

	if( desc.AddressV != ToD3D11TextureAddress( textureMode ) ) 
		return false;

	if( desc.AddressW != ToD3D11TextureAddress( textureMode ) )
		return false;


	return true;
}

