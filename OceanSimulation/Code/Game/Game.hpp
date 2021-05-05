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
class Material;
class GPUMesh;
class Clock;
class NamedProperties;
class WaveSimulation;
class FFTWaveSimulation;
class WaterObject;
struct Vertex_PCUTBN;
struct AABB3;
struct TextureCube;
struct ColorString;

typedef std::map<std::string, GPUMesh*>					TerrainMap;
typedef std::map<std::string, GPUMesh*>::iterator		TerrainMapIterator;
typedef std::map<std::string, std::string>				SimulationSettingMap;
typedef std::map<std::string, std::string>::iterator	SimulationSettingMapIterator;
typedef std::map<std::string, Material*>				WaterMaterialMap;
typedef std::map<std::string, Material*>::iterator		WaterMaterialMapIterator;

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

struct terrain_t
{
	float minHeight = 0.f;
	float maxHeight = 1.f;
	float padding[2];
};

struct water_t
{
	Vec2 NORMALS1_SCROLL_DIR = Vec2( 1.f, 0.f );
	Vec2 NORMALS2_SCROLL_DIR = Vec2( 0.f, 1.f );

	Vec2 NORMALS_SCROLL_SPEED = Vec2( 0.00f, 0.00f );
	float NEAR_PLANE = -0.9f;
	float FAR_PLANE = -100.f;

	Vec3 UPWELLING_COLOR = Vec3( 0.f, 0.2f, 0.3f );
	float SNELL_AIR_TO_WATER = 1.34f;

	float MAX_DEPTH = 25.f;
	float INVERSE_MAX_DEPTH = 0.04f;
	float FOAM_THICKNESS = 0.5f;
	float FOAMINESS = 0.4f;

	Mat44 WORLD_INVERSE_PROJECTION;
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
	float GetDeltaSeconds();
	void UpdateFromInput( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );
	void UpdateBasedOnMouseMovement();
	void UpdateSimulationFromInput();
	void UpdateWaterInfoFromInput();

	void CycleSelectedSimulationSettings();
	void CycleSelectedTerrain();
	void CycleSelectedMaterial();
	void UpdateTilingSize();
	void UpdateChoppiness();
	void UpdateTimeFactor();
	void UpdateSamples();
	void UpdateDimensions();
	void UpdateGlobalWaveAmplitude();
	void UpdateWaveSuppression();
	void UpdateWindSpeed();
	void UpdateWindDirection();
	void UpdateIWaveSourcePower();

	void UpdateFoamDepth();
	void UpdateFoaminess();
	void UpdateMaxDepth();

	void IncreaseSamples();
	void DecreaseSamples();
	void AddUniformDimensions( float dimensionsToAdd );
	void AddGlobalWaveAmp( float ampToAdd );
	void AddWaveSuppression( float suppressionToAdd );
	void AddWindSpeed( float windSpeedToAdd );
	void RotateWindDirByDegrees( float degreesToRotateBy );

	//Rendering
	void DrawWorldBasis() const;
	void DrawTerrain() const;
	void DrawWater() const;
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
	void AddIWaveSources();

	GPUMesh* CreateTerrainFromImage( char const* filepath, Vec2 const& meshDimensions, float minHeight, float maxHeight );
	void GenerateTerrainVerts( GPUMesh* meshToModify, IntVec2 const& vertDimensions, Vec2 const& dimensions, float minHeight, float maxHeight );

	Vec2 GetDimensionsForLongestColorSting( BitmapFont* font, float textHeight, std::vector<ColorString> const& strings ) const;
	std::string GetLongestColorSting( std::vector<ColorString> const& strings ) const;

	//Loading
	void LoadTerrains();
	void LoadSimulationSettings();
	void LoadMaterials();

	void AddTerrainByName( char const* name, GPUMesh* terrainMesh );
	void AddSimulationSettingByName( char const* name, char const* simulationFileName );
	void AddWaterMaterialByName( char const* name, Material* material );

	//Commands
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );
	void create_new_fft_simulation( EventArgs* args );
	void fft_from_xml( EventArgs* args );

	bool IsQuitting() const { return m_isQuitting; }

private:
	bool m_isDebugText = false;

	std::vector<GPUMesh*> m_terrains;
	std::vector<char const*> m_terrainNames;
	GPUMesh* m_selectedTerrain;
	int m_selectedTerrainIndex = 0;

	std::vector<char const*> m_simulationSettings;
	std::vector<char const*> m_simulationSettingNames;
	char const* m_selectedSettings;
	int m_selectedSettingsIndex = 0;
	std::string m_currentXML = "";

	std::vector<Material*> m_waterMaterials;
	std::vector<char const*> m_waterMaterialNames;
	Material* m_selectedMaterial;
	int m_selectedMaterialIndex = 0;

	bool m_showUnderwaterEffect = true;
	bool m_isSkyBox = true;
	bool m_updateCrate = true;
	bool m_scrollingNormals = false;
	bool m_isUnderWater = false;

	TextureCube* m_skyBoxTexture = nullptr;
	TextureCube* m_skyBox = nullptr;
	TextureCube* m_noSkyBox = nullptr;
	GPUMesh* m_skyCube = nullptr;

	water_t m_waterInfo;

	WaterObject* m_crate = nullptr;

	FFTWaveSimulation* m_FFTWaveSimulation = nullptr;
	float m_iWaveSourcePower = 1.f;

	Clock*		m_gameClock = nullptr;
	
	Light	m_theSun = Light::DIRECTIONAL;

	Rgba8 m_ambientColor = Rgba8( 255, 255, 255 );
	float m_ambientIntensity = 0.45f;

	Rgba8	m_clearColor = Rgba8::BLACK;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;


	Vec2	m_tempWindDir;
	Vec2	m_tempDimensions;
	uint	m_tempSamples			= 0;
	float	m_tempWindSpeed			= 0.f;
	float	m_tempGloabalWaveAmp	= 0.f;
	float	m_tempWaveSuppression	= 0.f;
};