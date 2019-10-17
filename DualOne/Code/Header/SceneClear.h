#pragma once

#include <memory>

#include "Scene.h"

class SceneClear : public Scene
{
public:
	class Impl;
private:
	std::unique_ptr<SceneClear::Impl> pImpl;
public:
	SceneClear();
	~SceneClear();
public:
	void	Init();
	void	Uninit();

	Result	Update( float elapsedTime );

	void	Draw( float elapsedTime );
private:
	bool	IsConrollerConnected() const;

	void	UpdateChooseItem();
	bool	IsDecisionTriggered() const;

	void	StartFade();

	Result	ReturnResult();
};
