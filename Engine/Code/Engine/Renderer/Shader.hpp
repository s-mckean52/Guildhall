#pragma once
#include <string>

class	RenderContext;
struct	buffer_attribute_t;
struct	ID3D11Resource;
struct	ID3D11VertexShader;
struct	ID3D11PixelShader;
struct	ID3D10Blob;
struct	ID3D11RasterizerState;
struct	ID3D11InputLayout;

enum ShaderType
{
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT,
};

class ShaderStage
{
public:
	~ShaderStage();
	bool Compile(	RenderContext* ctx,
					std::string const& filename, //Purely for debug reasons
					void const* source,
					size_t const sourceByteLen,
					ShaderType stage );

	bool IsValid() const	{ return ( m_handle != nullptr ); }

	void const* GetByteCode() const;
	size_t GetByteCodeLength() const;


public:
	ShaderType m_type;
	ID3D10Blob* m_byteCode = nullptr;
	union
	{
		ID3D11Resource		*m_handle;
		ID3D11VertexShader	*m_vs;
		ID3D11PixelShader	*m_fs;
	};
};

class Shader
{
public:
	Shader( RenderContext* context );
	~Shader();
	bool CreateFromFile( std::string const& filename );
	void CreateRasterState();

	const char*			GetFilePath() const;
	ID3D11InputLayout*	GetOrCreateInputLayout( buffer_attribute_t const* attribute );

public:
	const char* m_filePath = "";

	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;
	RenderContext* m_owner = nullptr;

	ID3D11RasterizerState* m_rasterState = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};

void* FileReadToNewBuffer( std::string const& filename, size_t *out_size );