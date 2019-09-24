#pragma once

#include "Scene.h"

/// <summary>
/// This is only show the rights logo.
/// </summary>
class SceneLogo : public Scene
{
private:
	enum class State
	{
		FADE_IN,
		WAIT,
		FADE_OUT,
		END
	};
private:
	State	status;
	size_t	sprRightsLogo;
	int		showIndex;		// 0-based.
	int		showCount;		// 1-based.
	int		timer;			// 0-based.
	float	alpha;			// 0.0f ~ 1.0f.
	float	scale;			// Use for magnification.
public:
	SceneLogo();
	~SceneLogo();
public:
	void	Init() override;
	void	Uninit() override;

	Result	Update( float elapsedTime ) override;

	void	Draw( float elapsedTime ) override;
private:
	Result	ReturnResult();
};