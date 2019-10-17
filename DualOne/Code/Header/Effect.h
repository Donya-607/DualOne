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
	Donya::Vector3 bossPos;
};

struct Particle
{
	const float GRAVITY = 0.196f;
	enum Type
	{
		NONE,
		SLED_EFFECT,
		BOSS_DAMAGE_EFFECT,
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
	Particle(Donya::Vector3 _emitterPos, Type _type, bool _noMove = false, float _scale = 1.0f);
	Particle(const Particle&);
	Particle& operator = (const Particle&);
	~Particle();

	// Update fanction
	void UpdateOfSleds();
	void UpdateOfMissiles();
	void UpdateOfBossDamage();
	void UpdateOfShockWave();

	//	Set Parameter fanction
	void SetNoneElements(Donya::Vector3 _emitterPos);
	void SetSledElements(Donya::Vector3 _emitterPos);
	void SetBossElements(Donya::Vector3 _emitterPos);
	void SetMissileElements(Donya::Vector3 _emitterPos, bool _noMove, float _scale = 1.0f);
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
	std::vector<Particle> bossDamageEffects;
	std::vector<Particle> shockWaveEffects;

	int				timer;

	// Explosion variable 
	bool			isExplosion;
	int				explosionPopNum;
	int				popNumOnce;
	Donya::Vector3	explosionPos;

	// BossDamage variable
	enum DLevel
	{
		LEVEL1,
		LEVEL2,
		LEVEL3,
	};
	bool			isBossSmoke;
	DLevel			damageLevel;

private:
	ParticleManager() :sprSled(nullptr), sprSmoke(nullptr), sledEffects(), missileEffects(), timer(0), popNumOnce(0), explosionPopNum(0), explosionPos(0.0f,0.0f,0.0f), isExplosion(false) , isBossSmoke(false) ,damageLevel(LEVEL1){}

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

	void DrawSmokeOfBoss
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
	void ReserveExplosionParticles(Donya::Vector3 _emitPos, int _popNum, int _onceNum);
	void CreateExplosionLoop();
	void CreateBossDamageParticle(Donya::Vector3 _pos);
	void CreateBossDamageLoop(Donya::Vector3 _pos);
	void StartBossDamageParticle();
	void UpdateBossDamageLevel();

	/*---------------------*/
	//	Judge erase fanction
	/*---------------------*/
private:
	void JudgeErase();
	void JudgeEraseSled();
	void JudgeEraseSmokeOfMissile();
	void JudgeEraseSmokeOfBoss();
	void JudgeEraseShockWave();

#if USE_IMGUI
	void UseImGui();
#endif

};