#pragma once

class Game;
class World;
class GPUMesh;
class PlayerStart;
class Camera;


class Map
{
public:
	Map( Game* theGame, World* theWorld );
	virtual ~Map();

	virtual void Update()			= 0;
	virtual void Render() const		= 0;

	virtual void SpawnPlayer( Camera* playerCamera );

protected:
	Game*		m_game		= nullptr;
	World*		m_world		= nullptr;
	GPUMesh*	m_mapMesh	= nullptr;

	PlayerStart* m_playerStart = nullptr;
};