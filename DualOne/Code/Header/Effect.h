#pragma once

#include <memory>
#include <vector>

#include "Donya/Vector.h"
#include "Donya/GeometricPrimitive.h"
#include "Donya/UseImgui.h"
#include "Donya/Template.h"

struct ParticleEmitterPosition
{
	Donya::Vector3 playerPos;
	Donya::Vector3 misslePos;
};

struct Particle
{
	const float GRAVITY = 0.196f;
	enum Type
	{
		NONE,
		SLED_EFFECT,
		BOSS_EFFECT,
		MISSLE_EFFECT,
	};
	Donya::Vector3			pos;
	Donya::Vector3			velocity;
	Donya::Vector3			startVelocity;
	Donya::Vector3			scale;
	static Donya::Vector3	setVelocity;
	float					angle;// degree
	int						existanceTime;
	Type					type;

	Particle();
	Particle(Donya::Vector3 _emitterPos, Type _type);
	Particle(const Particle&);
	Particle& operator = (const Particle&);
	~Particle();


	void SetSledElements(Donya::Vector3 _emitterPos);
	void SetMissleElements(Donya::Vector3 _emitterPos);

	void UpdateOfSleds();
	void UpdateOfMissles();


};

class ParticleManager : public Donya::Singleton<ParticleManager>
{
	friend Donya::Singleton<ParticleManager>;
	/*---------------------------------*/
	//	Assets
	/*---------------------------------*/
	std::unique_ptr<Donya::Geometric::TextureBoard>	sprSled;
	std::unique_ptr<Donya::Geometric::TextureBoard>	sprSmoke;
	/*---------------------------------*/

	/*---------------------------------*/
	//	Instance
	/*---------------------------------*/
	std::vector<Particle> sledEffects;
	std::vector<Particle> missleEffects;

	int				timer;

private:
	ParticleManager() :sprSled(nullptr), sprSmoke(nullptr), sledEffects(), missleEffects(), timer(0) {}

public:
	void Init();
	void Uninit();
	void Update(ParticleEmitterPosition _arg);
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

private:
	void LoadSprite();

	/*---------------------*/
	//	Draw fanction
	/*---------------------*/
public:
	void DrawSled
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true

	);

	void DrawSmokeOfMissle
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
	void CreateSmokeOfMissleParticle(Donya::Vector3 _pos);

	/*---------------------*/
	//	Create fanction
	/*---------------------*/
private:
	void JudgeEraseSled();

#ifdef USE_IMGUI
	void UseImGui();
#endif

};