#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Math/Vec3.hpp"

// struct Rgba8;
// class Shader;
// class Texture;
// class Sampler;
// class ShaderState;
// class RenderBuffer;


//---------------------------------------------------------------------------------------------------------
Material::Material( RenderContext* context, char const* filepath )
{
	m_context = context;
	m_filepath = filepath;
	m_ubo = new RenderBuffer( context, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	XmlDocument materialFile = new XmlDocument();
	materialFile.LoadFile( filepath );
	GUARANTEE_OR_DIE( materialFile.ErrorID() == 0, Stringf( "%s could not be loaded", filepath ).c_str() );
	SetFromXML( *materialFile.RootElement() );
}


//---------------------------------------------------------------------------------------------------------
Material::~Material()
{
	delete m_ubo;
	m_ubo = nullptr;
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
void Material::AddMaterialTexture( uint index, Texture* texture )
{
	uint newMaterialTexturesSize = index + 1;
	if( newMaterialTexturesSize > m_materialTexturesPerSlot.size() )
	{
		m_materialTexturesPerSlot.resize( newMaterialTexturesSize );
	}
	m_materialTexturesPerSlot[ index ] = texture;
}


//---------------------------------------------------------------------------------------------------------
void Material::AddSampler( Sampler* sampler )
{
	m_samplersPerSlot.push_back( sampler );
}


//---------------------------------------------------------------------------------------------------------
void Material::AddSampler( uint index, Sampler* sampler )
{
	uint newSamplersSize = index + 1;
	if( newSamplersSize > m_samplersPerSlot.size() )
	{
		m_samplersPerSlot.resize( newSamplersSize );
	}
	m_samplersPerSlot[ index ] = sampler;
}


//---------------------------------------------------------------------------------------------------------
void Material::UpdateUBOIfDirty()
{
	if( !m_isUBODirty || m_uboCPUData.size() == 0 ) return;

	m_ubo->Update( &m_uboCPUData[0], m_uboCPUData.size(), m_uboCPUData.size() );
}


//---------------------------------------------------------------------------------------------------------
void Material::SetFromXML( XmlElement const& element )
{
// 	< Material   name = "ship_dissolve"
// 		< ShaderState        filepath = "Data\Shaders\dissolve.shaderstate" / >
// 		<Tint               value = "255,255,255" / >
// 		<SpecularFactor     value = "0.0" / >
// 		<SpecularPower      value = "32.0" / >
// 		<DiffuseTexture     filepath = "Data\Models\scifi_fighter\diffuse.png" / >
// 		<MaterialTexture    filepath = "Data\Images\noise.png" / >
// 		<MaterialSampler    type = "linear" / >
// 
// 		<!--Material Data-->
// 		<float  name = "amount"           value = "0.0" / >
// 		<float  name = "edgeRange"        value = "0.1" / >
// 		<Vec3   name = "edgeStartColor"   value = "1.0,1.0,0.0" / >
// 		<Vec3   name = "edgeEndColor"     value = "1.0,0.0,0.0" / >
// 		< / Material>

	m_name = ParseXmlAttribute( element, "name", m_name );
	const XmlElement* nextElement = element.FirstChildElement();
	for( ;; )
	{
		if( nextElement == nullptr ) break;
		
		std::string elementTypeByName = nextElement->Name();

		if( elementTypeByName == "ShaderState" )			{ ParseShaderStateElement( *nextElement ); }
		else if( elementTypeByName == "Tint" )				{ m_tint = ParseXmlAttribute( *nextElement, "value", m_tint ); }
		else if( elementTypeByName == "SpecularFactor" )	{ m_specularFactor = ParseXmlAttribute( *nextElement, "value", m_specularFactor ); }
		else if( elementTypeByName == "SpecularPower" )		{ m_specularPower = ParseXmlAttribute( *nextElement, "value", m_specularPower ); }
		else if( elementTypeByName == "DiffuseTexture" )	{ ParseDiffuseTexture( *nextElement ); }
		else if( elementTypeByName == "NormalTexture" )		{ ParseNormalTexture( *nextElement ); }
		else if( elementTypeByName == "MaterialTexture" )	{ ParseMaterialTexture( *nextElement ); }
		else if( elementTypeByName == "MaterialSampler" )	{ ParseMaterialSampler( *nextElement ); }
		//else ParseDataElement( *nextElement );

		nextElement = nextElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
void Material::SetData( void const* data, size_t dataSize )
{
	m_uboCPUData.resize( dataSize );
	memcpy( &m_uboCPUData[0], data, dataSize );
	m_isUBODirty = true;
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseShaderStateElement( XmlElement const& element )
{
	std::string filepath = ParseXmlAttribute( element, "filepath", "" );
	
	if( filepath != "" )
	{
		SetShaderState( m_context->GetOrCreateShaderStateFromFile( filepath.c_str() ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseDiffuseTexture( XmlElement const& element )
{
	std::string filepath = ParseXmlAttribute( element, "filepath", "" );
	if( filepath != "" )
	{
		SetDiffuseTexture( m_context->CreateOrGetTextureFromFile( filepath.c_str() ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseNormalTexture( XmlElement const& element )
{
	std::string filepath = ParseXmlAttribute( element, "filepath", "" );
	if( filepath != "" )
	{
		SetNormalTexture( m_context->CreateOrGetTextureFromFile( filepath.c_str() ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseMaterialTexture( XmlElement const& element )
{
	std::string filepath = ParseXmlAttribute( element, "filepath", "" );
	if( filepath != "" )
	{
		AddMaterialTexture( m_context->CreateOrGetTextureFromFile( filepath.c_str() ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseMaterialSampler( XmlElement const& element )
{
	SamplerType samplerType;
	std::string type = ParseXmlAttribute( element, "type", "" );

	if( type == "linear" )		{ samplerType = SAMPLER_BILINEAR; }
	else if( type == "point" )	{ samplerType = SAMPLER_POINT; }
	else { return; }

	AddSampler( m_context->GetOrCreateSampler( samplerType ) );
}


//---------------------------------------------------------------------------------------------------------
void Material::ParseDataElement( XmlElement const& element )
{
	std::string dataType = element.Name();
	if( dataType == "float" ) 
	{
		m_isUBODirty = true;
		float dataToAppend = ParseFloatData( element );
		m_uboCPUData.push_back( (unsigned char)dataToAppend );
	}
	else if( dataType == "Vec3" )
	{
		m_isUBODirty = true;
		Vec3 dataToAppend = ParseVec3Data( element );
		m_uboCPUData.push_back( (unsigned char)dataToAppend.x );
		m_uboCPUData.push_back( (unsigned char)dataToAppend.y );
		m_uboCPUData.push_back( (unsigned char)dataToAppend.z );
	}
}


//---------------------------------------------------------------------------------------------------------
float Material::ParseFloatData( XmlElement const& element )
{
	float data = ParseXmlAttribute( element, "value", 0.f );
	return data;
}


//---------------------------------------------------------------------------------------------------------
Vec3 Material::ParseVec3Data( XmlElement const& element )
{
	Vec3 defaultValue = Vec3::ZERO;
	Vec3 data = ParseXmlAttribute( element, "value", defaultValue );
	return data;
}

