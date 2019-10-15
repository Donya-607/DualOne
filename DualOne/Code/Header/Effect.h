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
};

struct Particle
{
	const float GRAVITY = 0.196f;
	enum Type
	{
		NONE,
		SLED_EFFECT,
		BOSS_EFFECT,
		MISSILE_EFFECT,
		SHOCKWAVE_EFFECT,
	};
	/*----------------------------------------*/
	//	Variables
	/*----------------------------------------*/
	Donya::Vector3			pos;
	Donya::Vector3			velocity;
	Donya::Vector3			scale;
	static Donya::Vector3	setVelocity;
	Donya::Vector3			angle;
	Donya::Vector4			color;
	int						existanceTime;
	Type					type;

	/*----------------------------------------*/
	//	Fanctions
	/*----------------------------------------*/
	Particle();
	Particle(Donya::Vector3 _emitterPos, Type _type, bool _noMove = false);
	Particle(const Particle&);
	Particle& operator = (const Particle&);
	~Particle();

	// Update fanction
	void UpdateOfSleds();
	void UpdateOfMissiles();
	void UpdateOfShockWave();

	//	Set Parameter fanction
	void SetNoneElements(Donya::Vector3 _emitterPos);
	void SetSledElements(Donya::Vector3 _emitterPos);
	void SetBossElements(Donya::Vector3 _emitterPos);
	void SetMissileElements(Donya::Vector3 _emitterPos, bool _noMove);
	void SetShockWaveElements(Donya::Vector3 _emitterPos);

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
	std::vector<Particle> missileEffects;
	std::vector<Particle> shockWaveEffects;

	int				timer;

private:
	ParticleManager() :sprSled(nullptr), sprSmoke(nullptr), sledEffects(), missileEffects(), timer(0) {}

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

	void DrawSmokeOfMissile
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void DrawShockWave
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
	void CreateSmokeOfMissileParticle(Donya::Vector3 _pos);
	void CreateShockWaveParticle(Donya::Vector3 _pos);
	void CreateExplosionParticle(Donya::Vector3 _pos, int _loopNum);
	/*---------------------*/
	//	Judge erase fanction
	/*---------------------*/
private:
	void JudgeErase();
	void JudgeEraseSled();
	void JudgeEraseSmokeOfMissile();
	void JudgeEraseShockWave();

#if USE_IMGUI
	void UseImGui();
#endif

};