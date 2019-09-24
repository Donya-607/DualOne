#pragma once

#include <memory>

#include "Scene.h"

class SceneGame : public Scene
{
private:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
public:
	SceneGame();
	~SceneGame();
public:
	void	Init() override;
	void	Uninit() override;

	Result	Update( float elapsedTime ) override;

	void	Draw( float elapsedTime ) override;
private:
	void	InitPerStage( int stageNo, bool resetTimeAndBGM, bool restartFromRespawnPoint = false );

	void	DoCollisionDetection();

	Result	ReturnResult();
};
