#pragma once

#include <memory>

#include "SceneManager.h"

class Framework
{
private:
	std::unique_ptr<SceneMng> pSceneMng;
public:
	Framework();
	~Framework();
	Framework( const Framework &  ) = delete;
	Framework( const Framework && ) = delete;
	Framework &operator = ( const Framework &  ) = delete;
	Framework &operator = ( const Framework && ) = delete;
public:
	bool Init();
	void Uninit();

	void Update( float elapsed_time /* Elapsed seconds from last frame */ );

	void Draw( float elapsed_time /* Elapsed seconds from last frame */ );
private:
	bool LoadSounds();
	/// <summary>
	/// Valid only when Debug-mode.
	/// </summary>
	void DebugShowInformation();
};
