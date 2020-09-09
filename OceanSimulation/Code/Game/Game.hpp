#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>
#include <complex>

class Entity;
class XboxController;
class Shader;
class GPUMesh;
class Clock;
class NamedProperties;
struct Vertex_PCUTBN;
struct AABB3;

enum CameraViewOrientation
{
	RIGHT_HAND_X_RIGHT_Y_UP,
	RIGHT_HAND_X_FORWARD_Y_LEFT,
};

struct wave
{
public:
	float amplitude = 1.f;
	Vec3 direction = Vec3::UNIT_POSITIVE_X;
};


class Game
{
public:
	~Game();
	Game();

	//Basic Game
	void StartUp();
	void ShutDown();
	void Render();
	void Update();

	//Input
	void UpdateFromInput( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );
	void UpdateBasedOnMouseMovement();

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_FORWARD_Y_LEFT );
	void RenderWorld() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;
	
	//Other
	void AddTestCubeToIndexVertexArray( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indexArray, AABB3 const& box, Rgba8 const& color );

	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );

	bool IsQuitting() const { return m_isQuitting; }
	void PlayTestSound();

	void GenerateOceanSurface( std::vector<Vertex_PCUTBN>& verts, std::vector<uint>& indicies, Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps );
	Vec3 GetWaveHeightAtPosition( Vec3 const& position );
	void UpdateSurfaceMesh();
	float Phillips( Vec3 const& waveDir );

	void UpdateGerstnerWave();
	Vec3 GetWaveVectorSums(Vec3 const& initialPosition, std::vector<Vec3> const& waveVectors);

private:
	Clock* m_gameClock = nullptr;

	GPUMesh*	m_meshCube		= nullptr;
	Transform*	m_cubeTransform	= nullptr;
	Texture*	m_test			= nullptr;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 1.f;

	float m_distanceFromCamera = -1.f;;

	Shader* m_testShader	= nullptr;

	SoundID m_testSound;

	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;

	Rgba8	m_clearColor = Rgba8::BLACK;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;

	bool m_isFogEnabled = true;

	std::vector<uint>			m_surfacePlaneIndicies;
	std::vector<Vertex_PCUTBN>	m_surfacePlaneVerts;
};