#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/buffer_attribute_t.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <d3dcompiler.h>


//---------------------------------------------------------------------------------------------------------
static char const* GetDefaultEntryPointForStage( ShaderType type )
{
	switch( type )
	{
	case SHADER_TYPE_VERTEX:
		return "VertexFunction";
	case SHADER_TYPE_FRAGMENT:
		return "FragmentFunction";
	default:
		GUARANTEE_OR_DIE( false, "Bad Stage" );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
static char const* GetShaderModelForStage( ShaderType type )
{
	switch( type )
	{
	case SHADER_TYPE_VERTEX:
		return "vs_5_0";
	case SHADER_TYPE_FRAGMENT:
		return "ps_5_0";
	default:
		GUARANTEE_OR_DIE( false, "Unknown shader stage" );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
ShaderStage::~ShaderStage()
{
	DX_SAFE_RELEASE( m_byteCode );
	DX_SAFE_RELEASE( m_handle );
}


//---------------------------------------------------------------------------------------------------------
bool ShaderStage::Compile( RenderContext* ctx, std::string const& filename, void const* source, size_t const sourceByteLen, ShaderType stage )
{
	if( IsValid() )
	{
		DX_SAFE_RELEASE( m_byteCode );
		DX_SAFE_RELEASE( m_handle );
	}

	char const* entrypoint = GetDefaultEntryPointForStage( stage );
	char const* shaderModel = GetShaderModelForStage( stage );
	
	DWORD compileFlags = 0U;
	#if defined( DEBUG_SHADERS )
		compile_flags |= D3DCOMPILE_DEBUG;
		compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;   // cause, FIX YOUR WARNINGS
	#else 
		// compile_flags |= D3DCOMPILE_SKIP_VALIDATION;       // Only do this if you know for a fact this shader works with this device (so second run through of a game)
		compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;   // Yay, fastness (default is level 1)
	#endif

		ID3DBlob* byteCode	= nullptr;
		ID3DBlob* errors	= nullptr;

	HRESULT hr = ::D3DCompile( source,
		sourceByteLen,						// plain text source code
		filename.c_str(),                   // optional, used for error messages (If you HLSL has includes - it will not use the includes names, it will use this name)
		nullptr,                            // pre-compiler defines - used more for compiling multiple versions of a single shader (different quality specs, or shaders that are mostly the same outside some constants)
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  // include rules - this allows #includes in the shader to work relative to the src_file path or my current working directly
		entrypoint,                         // Entry Point for this shader
		shaderModel,                        // Compile Target (MSDN - "Specifying Compiler Targets")
		compileFlags,						// Flags that control compilation
		0,                                  // Effect Flags (we will not be doing Effect Files)
		&byteCode,							// [OUT] ID3DBlob (buffer) that will store the byte code.
		&errors );                          // [OUT] ID3DBlob (buffer) that will store error information


	if( FAILED( hr ) )
	{
		if( errors != nullptr ) {
			char* errorString = (char*) errors->GetBufferPointer();
			DebuggerPrintf( "Failed to compile [%s].  Compiler gave the following output;\n%s",
				filename.c_str(),
				errorString );
			DX_SAFE_RELEASE( errors );

			//DEBUGBREAK();
		}
		else {
			DebuggerPrintf( "Failed with HRESULT: %u", hr );
		}
		return false;
	}
	else
	{
		ID3D11Device* device = ctx->m_device;
		void const* byteCodePtr = byteCode->GetBufferPointer();
		size_t byteCodeSize = byteCode->GetBufferSize();

		switch( stage )
		{
		case SHADER_TYPE_VERTEX:
			hr = device->CreateVertexShader( byteCodePtr, byteCodeSize, nullptr, &m_vs );
			GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to link shader stage" );
			break;
		case SHADER_TYPE_FRAGMENT:
			hr = device->CreatePixelShader( byteCodePtr, byteCodeSize, nullptr, &m_fs );
			GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to link shader stage" );
			break;
		default:
			GUARANTEE_OR_DIE( false, "Unimplemented Stage" );
			break;
		}
	}

	DX_SAFE_RELEASE( errors );

	if( stage == SHADER_TYPE_VERTEX )
	{
		m_byteCode = byteCode;
	}
	else
	{
		DX_SAFE_RELEASE( byteCode );
		m_byteCode = nullptr;
	}

	m_type = stage;

	return IsValid();
}


//---------------------------------------------------------------------------------------------------------
void const* ShaderStage::GetByteCode() const
{
	return m_byteCode->GetBufferPointer();
}


//---------------------------------------------------------------------------------------------------------
size_t ShaderStage::GetByteCodeLength() const
{
	return m_byteCode->GetBufferSize();
}

//---------------------------------------------------------------------------------------------------------
Shader::Shader( RenderContext* context )
	: m_owner( context )
{
}


//---------------------------------------------------------------------------------------------------------
Shader::~Shader()
{
	DX_SAFE_RELEASE( m_inputLayout );
}


//---------------------------------------------------------------------------------------------------------
bool Shader::Recompile()
{
	size_t file_size = 0;
	void* source = FileReadToNewBuffer( m_filePath, &file_size );
	if( source == nullptr )
	{
		return false;
	}

	m_vertexStage.Compile( m_owner, m_filePath, source, file_size, SHADER_TYPE_VERTEX );
	m_fragmentStage.Compile( m_owner, m_filePath, source, file_size, SHADER_TYPE_FRAGMENT );

	delete[] source;

	return m_vertexStage.IsValid() && m_fragmentStage.IsValid();
}


//---------------------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( std::string const& filename )
{
	m_filePath = filename;
	size_t file_size = 0;
	void* source = FileReadToNewBuffer( filename, &file_size );
	if( source == nullptr )
	{
		return false;
	}

	m_vertexStage.Compile( m_owner, filename, source, file_size, SHADER_TYPE_VERTEX );
	m_fragmentStage.Compile( m_owner, filename, source, file_size, SHADER_TYPE_FRAGMENT );

	delete[] source;

	return m_vertexStage.IsValid() && m_fragmentStage.IsValid();
}


bool Shader::CreateFromSourceCode( const char* sourceCode )
{
	size_t size = strlen( sourceCode );
	void* source = (void*)sourceCode;
	   
	m_vertexStage.Compile( m_owner, "Created From Source", source, size, SHADER_TYPE_VERTEX );
	m_fragmentStage.Compile( m_owner, "Created From Source", source, size, SHADER_TYPE_FRAGMENT );

	return m_vertexStage.IsValid() && m_fragmentStage.IsValid();
}


//---------------------------------------------------------------------------------------------------------
const char* Shader::GetFilePath() const
{
	return m_filePath.c_str();
}


//---------------------------------------------------------------------------------------------------------
ID3D11InputLayout* Shader::GetOrCreateInputLayout( buffer_attribute_t const* attribute )
{
	if( m_lastBoundLayout == attribute )
	{
		return m_inputLayout;
	}
	
	DX_SAFE_RELEASE( m_inputLayout );

	int vertDescIndex = 0;
	D3D11_INPUT_ELEMENT_DESC newVertexAttribute;
	std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDescription;
	for( ;; )
	{
		const buffer_attribute_t* currentAttribute = &attribute[ vertDescIndex ];

		if( currentAttribute->IsDefault() ) break;

		newVertexAttribute.SemanticName				= currentAttribute->name.c_str();
		newVertexAttribute.SemanticIndex			= 0;
		newVertexAttribute.InputSlot				= 0;
		newVertexAttribute.AlignedByteOffset		= currentAttribute->offset;
		newVertexAttribute.InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		newVertexAttribute.InstanceDataStepRate		= 0;


		switch( currentAttribute->type )
		{
		case BUFFER_FORMAT_VEC3:
			newVertexAttribute.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case BUFFER_FORMAT_VEC2:
			newVertexAttribute.Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case BUFFER_FORMAT_R8G8B8A8_UNORM:
			newVertexAttribute.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		default:
			ERROR_AND_DIE( "Unknown vertex description format" );
			break;
		}

		vertexDescription.push_back( newVertexAttribute );
		++vertDescIndex;
	}

	//Create the input
	ID3D11Device* device = m_owner->m_device;
	device->CreateInputLayout(
		&vertexDescription[0], static_cast<unsigned int>( vertexDescription.size() ),
		m_vertexStage.GetByteCode(), m_vertexStage.GetByteCodeLength(),
		&m_inputLayout );

	return m_inputLayout;
}


//---------------------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( std::string const& filename, size_t *out_size )
{
	FILE* fp = nullptr;
	fopen_s( &fp, filename.c_str(), "r" );
	if( fp == nullptr )
	{
		return nullptr;
	}

	fseek( fp, 0, SEEK_END );
	long file_size = ftell( fp ) + 1;

	unsigned char* buffer = new unsigned char[ file_size ];
	if( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		size_t bytes_read = fread( buffer, 1, file_size, fp );
		buffer[ bytes_read ] = NULL;

		if( out_size != nullptr )
		{
			*out_size = (size_t)bytes_read;
		}
	}

	fclose( fp );
	return buffer;
}
