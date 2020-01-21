#pragma once


struct MapTileData
{
public:
	bool m_isAccessible = false;
	bool m_hasBeenProcessed = false;

public:
	~MapTileData() {};
	MapTileData() {};
};
