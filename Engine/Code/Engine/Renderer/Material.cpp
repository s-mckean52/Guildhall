#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"

// struct Rgba8;
// class Shader;
// class Texture;
// class Sampler;
// class ShaderState;
// class RenderBuffer;


//---------------------------------------------------------------------------------------------------------
Material::Material( RenderContext* context )
{
	m_context = context;
	m_ubo = new RenderBuffer( context, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//---------------------------------------------------------------------------------------------------------
Material::~Material()
{
}


//---------------------------------------------------------------------------------------------------------
void Material::SetShaderState( ShaderState* shaderState )
{
	m_shaderState = shaderState;
}


//---------------------------------------------------------------------------------------------------------
void Material::SetDiffuseTexture( Texture* texture )
{
	m_diffuseTexture = texture;
}


//---------------------------------------------------------------------------------------------------------
void Material::SetNormalTexture( Texture* texture )
{
	m_normalTexture = texture;
}


//---------------------------------------------------------------------------------------------------------
void Material::AddMaterialTexture( Texture* texture )
{
	m_materialTexturesPerSlot.push_back( texture );
}


//---------------------------------------------------------------------------------------------------------
void Material::AddSampler( Sampler* sampler )
{
	m_samplersPerSlot.push_back( sampler );
}


//---------------------------------------------------------------------------------------------------------
void Material::UpdateUBOIfDirty()
{
	if( !m_isUBODirty ) return;

	m_ubo->Update( &m_uboCPUData[0], m_uboCPUData.size(), m_uboCPUData.size() );
}

