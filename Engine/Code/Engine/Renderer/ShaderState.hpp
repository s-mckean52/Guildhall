#pragma once
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>

class RenderContext;
class Shader;

class ShaderState
{
public:
	ShaderState( RenderContext* context, char const* filepath );
	~ShaderState();

	void SetFromXml( XmlElement const& xmlElement );

	std::string	GetFilePath() const						{ return m_filepath; }
	std::string	GetName() const							{ return m_name; }
	Shader*		GetShader() const						{ return m_shader; }
	BlendMode	GetBlendMode() const					{ return m_blendMode; }
	CullMode	GetCullMode() const						{ return m_cullMode; }
	FillMode	GetFillMode() const						{ return m_fillMode; }
	CompareFunc	GetCompareMode() const					{ return m_compareMode; }
	bool		IsWriteOnDepth() const					{ return m_doesWriteDepth; }
	bool		IsCounterClockwiseWindorder() const		{ return m_isCounterClockwiseWindOrder; }


private:
	BlendMode	GetBlendModeFromXml( XmlElement const& xmlElement );
	CullMode	GetCullModeFromXml( XmlElement const& xmlElement );
	FillMode	GetFillModeFromXML( XmlElement const& xmlElement );
	CompareFunc	GetCompareModeFromXML(XmlElement const& xmlElement );


private:
	std::string m_filepath	= "";
	std::string m_name		= "";

	RenderContext* m_context	= nullptr;
	Shader* m_shader			= nullptr;

	BlendMode m_blendMode				= BlendMode::ALPHA;
	CullMode m_cullMode					= CULL_MODE_BACK;
	FillMode m_fillMode					= FILL_MODE_SOLID;
	CompareFunc m_compareMode			= COMPARE_FUNC_LEQUAL;
	bool m_doesWriteDepth				= true;
	bool m_isCounterClockwiseWindOrder	= true;
};