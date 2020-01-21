#pragma once

class App
{
private:
	bool m_isQuitting = false;

public:
	App() {};
	~App() {};
	void StartUp();
	void ShutDown();
	void RunFrame();
	void RestartGame();

	bool HandleQuitRequested();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
};