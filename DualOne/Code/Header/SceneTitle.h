#pragma once

#include <memory>

#include "Scene.h"

class SceneTitle : public Scene
{
private:
	class Impl;
	std::unique_ptr<SceneTitle::Impl> pImpl;
public:
	SceneTitle();
	~SceneTitle();
public:
	void	Init();
	void	Uninit();

	Result	Update( float elapsedTime );

	void	Draw( float elapsedTime );
private:
	void	StartFade();

	void	TitleUpdate();
	void	SelectUpdate();

	void	TitleDraw();
	void	SelectDraw();

	void	Scroll();
	void	ChooseStage();

	void	StoreStageNo();

	Result	ReturnResult();
};
