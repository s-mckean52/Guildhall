#pragma once

class TileDefinition;

struct TileMetaData
{
public:
	TileDefinition* m_tileTypeToChangeTo = nullptr;

public:
	TileMetaData() = default;
};