#pragma once

#include <memory>

#include "Scene.h"

class SceneOver : public Scene
{
private:
	class Impl;
	std::unique_ptr<SceneOver::Impl> pImpl;
public:
	SceneOver();
	~SceneOver();
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
