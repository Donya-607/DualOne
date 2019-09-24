#pragma once

#include "Animation.h"
#include "GamepadXInput.h"
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
	SpriteSheet		sprUI;
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
