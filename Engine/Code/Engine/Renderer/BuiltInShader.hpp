#pragma once

struct BuiltInShader
{
public:
	const char* m_sourceCode = "";

public:
	BuiltInShader( const char* sourceCode );

	static BuiltInShader BUILT_IN_DEFAULT;
	static BuiltInShader BUILT_IN_ERROR;
};