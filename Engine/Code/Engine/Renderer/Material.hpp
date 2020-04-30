#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <vector>
#include <string>


struct Rgba8;
struct Texture;
struct Vec3;
class Shader;
class Sampler;
class ShaderState;
class RenderBuffer;
class RenderContext;


class Material
{
public:
	Material( RenderContext* context, char const* filepath );
	~Material();

	void SetShaderState( ShaderState* shaderState );
	void SetDiffuseTexture( Texture* texture );
	void SetNormalTexture( Texture* texture );
	void AddMaterialTexture( Texture* texture );
	void AddSampler( Sampler* sampler );
	void UpdateUBOIfDirty();

	void SetFromXML( XmlElement const& element );
	
public:
	//Templates
	void SetData( void const* data, size_t dataSize )
	{
		m_uboCPUData.resize(dataSize);
		memcpy(&m_uboCPUData[0], data, dataSize);
		m_isUBODirty = true;
	}

	template<typename UBO_STRUCT_TYPE>
	void SetData( UBO_STRUCT_TYPE const& data );

	template<typename UBO_STRUCT_TYPE>
	UBO_STRUCT_TYPE* GetDataAs();


private:
	// Child Element Parse
	void ParseShaderStateElement( XmlElement const& element );
	void ParseDiffuseTexture( XmlElement const& element );
	void ParseNormalTexture( XmlElement const& element );
	void ParseMaterialTexture( XmlElement const& element );
	void ParseMaterialSampler( XmlElement const& element );
	void ParseDataElement( XmlElement const& element );

	//Data Element Parse
	float ParseFloatData( XmlElement const& element );
	Vec3 ParseVec3Data( XmlElement const& element );


public:
	std::string m_filepath = "";
	std::string m_name = "";

	RenderContext* m_context = nullptr;
	ShaderState* m_shaderState = nullptr;

	Rgba8 m_tint = Rgba8::WHITE;
	float m_specularFactor = 0.0f;
	float m_specularPower = 32.0f;

	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	std::vector<Texture*> m_materialTexturesPerSlot;
	std::vector<Sampler*> m_samplersPerSlot;

	std::vector<unsigned char> m_uboCPUData;
	RenderBuffer* m_ubo = nullptr;

private:
	bool m_isUBODirty = true;
};


//---------------------------------------------------------------------------------------------------------
template<typename UBO_STRUCT_TYPE>
void Material::SetData( UBO_STRUCT_TYPE const& data )
{
	SetData( &data, sizeof( UBO_STRUCT_TYPE ) );
}


//---------------------------------------------------------------------------------------------------------
template<typename UBO_STRUCT_TYPE>
UBO_STRUCT_TYPE* Material::GetDataAs()
{
	m_isUBODirty = true;
	if( m_uboCPUData.size() == sizeof( UBO_STRUCT_TYPE ) )
	{
		return (UBO_STRUCT_TYPE*) &m_uboCPUData[0];
	}
	else
	{
		m_uboCPUData.resize( sizeof( UBO_STRUCT_TYPE ) );
		UBO_STRUCT_TYPE* retPtr = (UBO_STRUCT_TYPE*) &m_uboCPUData[0];
		new (retPtr)UBO_STRUCT_TYPE();

		return retPtr;
	}
}