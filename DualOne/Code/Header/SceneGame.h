#pragma once

#include <memory>
#include <vector>

#include "Scene.h"
#include "Donya/GeometricPrimitive.h"

class SceneGame : public Scene
{
public:
	struct Impl;
private:
	std::unique_ptr<Impl> pImpl;
	Donya::Geometric::Sphere sphere;
	std::vector<Donya::Geometric::Cube> ground;
public:
	SceneGame();
	~SceneGame();
public:
	void	Init() override;
	void	Uninit() override;

	Result	Update( float elapsedTime ) override;

	void	Draw( float elapsedTime ) override;
private:
	/// <summary>
	/// This method is linking to title-scene's method.
	/// </summary>
	bool	IsDecisionTriggered() const;

	void	UpdateCamera();

	/// <summary>
	/// Check the collision between each objects, and call appropriate method.
	/// </summary>
	void	DetectCollision();

	Result	ReturnResult();
};
