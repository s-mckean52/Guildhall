#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
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

			DEBUGBREAK();
		}
		else {
			DebuggerPrintf( "Failed with HRESULT: %u", hr );
		}
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
Shader::Shader( RenderContext* context )
	: m_owner( context )
{
	CreateRasterState();
}


//---------------------------------------------------------------------------------------------------------
Shader::~Shader()
{
	DX_SAFE_RELEASE( m_rasterState );
}


//---------------------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( std::string const& filename )
{
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


//---------------------------------------------------------------------------------------------------------
void Shader::CreateRasterState()
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = D3D11_FILL_SOLID;	//FIlled Triangle
	desc.CullMode = D3D11_CULL_NONE;
	desc.FrontCounterClockwise = TRUE;	// the only reason we're doing this; 
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	ID3D11Device* device = m_owner->m_device;
	device->CreateRasterizerState( &desc, &m_rasterState );
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
