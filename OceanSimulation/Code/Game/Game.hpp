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
#include "Engine/Renderer/Light.hpp"
#include <vector>
#include <complex>

class Entity;
class XboxController;
class Shader;
class GPUMesh;
class Clock;
class NamedProperties;
class WaveSimulation;
class FFTWaveSimulation;
class WaterObject;
struct Vertex_PCUTBN;
struct AABB3;
struct TextureCube;

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
	void UpdateSimulationFromInput();

	void IncreaseSamples();
	void DecreaseSamples();
	void AddUniformDimensions( float dimensionsToAdd );
	void AddGlobalWaveAmp( float ampToAdd );
	void AddWaveSuppression( float suppressionToAdd );
	void AddWindSpeed( float windSpeedToAdd );
	void RotateWindDirByDegrees( float degreesToRotateBy );

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_FORWARD_Y_LEFT );
	void RenderWorld() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;
	
	//Other
	Clock* GetGameClock() const { return m_gameClock; }
	void AddTestCubeToIndexVertexArray( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indexArray, AABB3 const& box, Rgba8 const& color );
	void CreateNewFFTSimulation( int samples, Vec2 const& dimensions, float windSpeed );
	void LoadSimulationFromXML( char const* filepath );
	void ReloadCurrentXML();
	void LoadCurrentTempValues();
	void SetTempValues();

	//Commands
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );
	void create_new_fft_simulation( EventArgs* args );
	void fft_from_xml( EventArgs* args );

	bool IsQuitting() const { return m_isQuitting; }
	void PlayTestSound();

private:
	TextureCube* m_skyBox = nullptr;
	GPUMesh* m_skyCube = nullptr;

	WaterObject* m_testCube = nullptr;

	char const* m_currentXML = "";

	WaveSimulation* m_DFTWaveSimulation = nullptr;
	FFTWaveSimulation* m_FFTWaveSimulation = nullptr;

	Clock*		m_gameClock = nullptr;
	Texture*	m_test = nullptr;
	
	Light	m_theSun = Light::DIRECTIONAL;

	Rgba8 m_ambientColor = Rgba8( 255, 255, 204 );
	float m_ambientIntensity = 0.0f;

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

	int m_selectedWaveIndex = 0;

	Vec2	m_tempWindDir;
	Vec2	m_tempDimensions;
	uint	m_tempSamples			= 0;
	float	m_tempWindSpeed			= 0.f;
	float	m_tempGloabalWaveAmp	= 0.f;
	float	m_tempWaveSuppression	= 0.f;
};