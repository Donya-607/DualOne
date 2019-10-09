#pragma once

#include <memory>
#include <vector>

#include "Donya/Vector.h"
#include "Donya/GeometricPrimitive.h"

struct Particle
{
	const float GRAVITY = 0.1f;
	enum Type
	{
		NONE,
		SLED_EFFECT,
		BOSS_EFFECT,
		MISSLE_EFFECT,
	};
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
	int				existanceTime;
	Type			type;

	Particle();
	Particle(Donya::Vector3 _emitterPos, Type _type);
	~Particle();

	void Update();
};

class ParticleManager
{
	/*---------------------------------*/
	//	Assets
	/*---------------------------------*/
	std::unique_ptr<Donya::Geometric::TextureBoard>	sprSled;
	std::unique_ptr<Donya::Geometric::TextureBoard> sprBoss;
	std::unique_ptr<Donya::Geometric::TextureBoard> sprMissle;
	/*---------------------------------*/
	std::vector<Particle> sledEffect;

	int												timer;

public:
	void Init();
	void Uninit();
	void Update();
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void LoadSprite();
	void DrawSled
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);


	/*---------------------*/
	//	Create fanction
	/*---------------------*/
	void CreateSledParticle(Donya::Vector3 _pos);

	/*---------------------*/
	//	Create fanction
	/*---------------------*/
	void JudgeEraseSled();

};