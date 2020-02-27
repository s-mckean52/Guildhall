#pragma once
#include <string>

struct BuiltInShader
{
public:
	std::string m_name = "";
	const char* m_sourceCode = "";

public:
	BuiltInShader( std::string name, const char* sourceCode );

	static const char* BUILT_IN_DEFAULT;
	static const char* BUILT_IN_ERROR;
};