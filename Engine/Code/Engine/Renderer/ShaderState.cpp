#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
ShaderState::ShaderState( RenderContext* context, XmlElement const& xmlElement )
{
	m_context = context;
	SetFromXml( xmlElement );
}


//---------------------------------------------------------------------------------------------------------
ShaderState::~ShaderState()
{
}


//---------------------------------------------------------------------------------------------------------
void ShaderState::SetFromXml( XmlElement const& xmlElement )
{
	m_name = ParseXmlAttribute( xmlElement, "Name", m_name );

	std::string shaderFilePath = ParseXmlAttribute( xmlElement, "Shader", "" );
	if( shaderFilePath != "" )
	{
		m_shader = m_context->GetOrCreateShader( shaderFilePath.c_str() );
	}

	m_blendMode		= GetBlendModeFromXml( xmlElement );
	m_cullMode		= GetCullModeFromXml( xmlElement );
	m_fillMode		= GetFillModeFromXML( xmlElement );
	m_compareMode	= GetCompareModeFromXML( xmlElement );

	m_doesWriteDepth = ParseXmlAttribute( xmlElement, "WriteOnDepth", m_doesWriteDepth );
	m_isCounterClockwiseWindOrder = ParseXmlAttribute( xmlElement, "CounterClockwiseWindOrder", m_isCounterClockwiseWindOrder );
}


//---------------------------------------------------------------------------------------------------------
BlendMode ShaderState::GetBlendModeFromXml( XmlElement const& xmlElement )
{
	std::string blendModeAsString = ParseXmlAttribute( xmlElement, "BlendMode", "" );

	if( blendModeAsString == "Additive" )		{ return BlendMode::ADDITIVE; }
	else if( blendModeAsString == "Alpha" )		{ return BlendMode::ALPHA; }
	else if( blendModeAsString == "Disabled" )	{ return BlendMode::DISABLED; }

	return m_blendMode;
}


//---------------------------------------------------------------------------------------------------------
CullMode ShaderState::GetCullModeFromXml( XmlElement const& xmlElement )
{
	std::string cullModeAsString = ParseXmlAttribute( xmlElement, "CullMode", "" );

	if( cullModeAsString == "Back" )		{ return CULL_MODE_BACK; }
	else if( cullModeAsString == "Front" )	{ return CULL_MODE_FRONT; }
	else if( cullModeAsString == "None" )	{ return CULL_MODE_NONE; }

	return m_cullMode;
}


//---------------------------------------------------------------------------------------------------------
FillMode ShaderState::GetFillModeFromXML( XmlElement const& xmlElement )
{
	std::string fillModeAsString = ParseXmlAttribute( xmlElement, "FillMode", "" );

	if( fillModeAsString == "Wireframe" )	{ return FILL_MODE_WIREFRAME; }
	else if( fillModeAsString == "Solid" )	{ return FILL_MODE_SOLID; }

	return m_fillMode;
}


//---------------------------------------------------------------------------------------------------------
CompareFunc ShaderState::GetCompareModeFromXML( XmlElement const& xmlElement ) 
{
	std::string compareModeAsString = ParseXmlAttribute( xmlElement, "CompareMode", "" );

	if( compareModeAsString == "LessThanOrEqual" )			{ return COMPARE_FUNC_LEQUAL; }
	else if( compareModeAsString == "GreaterThanOrEqual" )	{ return COMPARE_FUNC_GEQUAL; }
	else if( compareModeAsString == "Always" )				{ return COMPARE_FUNC_ALWAYS; }
	else if( compareModeAsString == "Equal" )				{ return COMPARE_FUNC_EQUAL; }
	else if( compareModeAsString == "Never" )				{ return COMPARE_FUNC_NEVER; }

	return m_compareMode;
}

