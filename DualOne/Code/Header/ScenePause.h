#pragma once

#include "Donya/GamepadXInput.h"

#include "Scene.h"

class ScenePause : public Scene
{
private:
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		Resume,
		ReTry,
	};
private:
	Choice			choice;
	size_t			sprFont;
	Donya::XInput	controller;
public:
	ScenePause();
	~ScenePause();
public:
	void	Init();
	void	Uninit();

	Result	Update( float elapsedTime );

	void	Draw( float elapsedTime );
private:
	void	UpdateChooseItem();

	Result	ReturnResult();
};
