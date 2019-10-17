#include "Effect.h"

#include "Donya/Random.h"
#include "Donya/Blend.h"

#include "FilePath.h"

Donya::Vector3	Particle::setVelocity;
/*-------------------------------------------------*/
//
//	ParticleManager
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	初期化関数
/*-------------------------------------------------*/
void ParticleManager::Init()
{
	LoadSprite();
	timer = 0;
	isExplosion = false;
	isSlide = true;
	explosionPopNum = 0;
	popNumOnce = 0;
	explosionPos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	isBossSmoke = false;
	damageLevel = LEVEL1;
}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void ParticleManager::Update(ParticleEmitterPosition _arg)
{
	++timer;
	if (isSlide)
	{
		for (int i = 0; i < 3; i++)
		{
			CreateSledParticle(_arg.playerPos);
		}
	}

	// Explosion!
	CreateExplosionLoop();

	CreateBossDamageLoop(_arg.bossPos);

	JudgeErase();

	for (auto& it : sledEffects)
	{
		it.UpdateOfSleds();
	}
	for (auto& it : missileEffects)
	{
		it.UpdateOfMissiles();
	}
	for (auto& it : bossDamageEffects)
	{
		it.UpdateOfBossDamage();
	}
	for (auto& it : shockWaveEffects)
	{
		it.UpdateOfShockWave();
	}


#ifdef USE_IMGUI
//	UseImGui();
#endif
}

/*-------------------------------------------------*/
//	描画関数
/*-------------------------------------------------*/
void ParticleManager::Draw(const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	Donya::Blend::Set(Donya::Blend::Mode::ADD);
	DrawSled(matView, matProjection, lightDirection, cameraPosition, isEnableFill);
	DrawSmokeOfMissile(matView, matProjection, lightDirection, cameraPosition, isEnableFill);
	DrawShockWave(matView, matProjection, lightDirection, cameraPosition, isEnableFill);
	DrawSmokeOfBoss( matView, matProjection, lightDirection, cameraPosition, isEnableFill );
	Donya::Blend::Set(Donya::Blend::Mode::ALPHA);
}

/*-------------------------------------------------*/
//	終了関数
/*-------------------------------------------------*/
void ParticleManager::Uninit()
{
	sprSled.reset( nullptr );
	sprSmoke.reset( nullptr );

	std::vector<Particle>().swap( sledEffects );
	std::vector<Particle>().swap( missileEffects );
	std::vector<Particle>().swap( bossDamageEffects );
	std::vector<Particle>().swap( shockWaveEffects );
}

/*-------------------------------------------------*/
//	Spriteのロード関数
/*-------------------------------------------------*/
void ParticleManager::LoadSprite()
{
	sprSled = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::SledEffect));
	sprSled->Init();

	sprSmoke = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::SmokeWhiteEffect));
	sprSmoke->Init();
}

/*-------------------------------------------------*/
//	Sledの描画関数
/*-------------------------------------------------*/
void ParticleManager::DrawSled(const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	using namespace DirectX;

	auto Matrix = [](const XMFLOAT4X4 & matrix)
	{
		return XMLoadFloat4x4(&matrix);
	};
	auto Float4x4 = [](const XMMATRIX & M)
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4(&matrix, M);
		return matrix;
	};

	DirectX::XMFLOAT4X4		view2 = matView;
	DirectX::XMMATRIX		inv_view2;
	view2._14 = view2._24 = view2._34 = 0.0f;
	view2._41 = 0.0f; view2._42 = 0.0f; 							//	位置情報だけを削除
	view2._43 = 0.0f; view2._44 = 1.0f;								//	
	inv_view2 = DirectX::XMLoadFloat4x4(&view2);					//	Matrix型へ再変換
	inv_view2 = DirectX::XMMatrixInverse(nullptr, inv_view2);		//	view行列の逆行列作成

	//ビュー行列、投影行列を合成した行列の作成
	DirectX::XMMATRIX	VPSynthesisMatrix = Matrix(matView) * Matrix(matProjection);

	XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	for (auto& it : sledEffects)
	{
		if (it.existanceTime <= 10)
		{
			color.w = static_cast<float>( (it.existanceTime - 1)/10 );
		}
		else
		{
			color.w = 1.0f;
		}
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;

		sprSled->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}

void ParticleManager::DrawSmokeOfMissile
(
	const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	using namespace DirectX;

	auto Matrix = [](const XMFLOAT4X4 & matrix)
	{
		return XMLoadFloat4x4(&matrix);
	};
	auto Float4x4 = [](const XMMATRIX & M)
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4(&matrix, M);
		return matrix;
	};

	for (auto& it : missileEffects)
	{
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(it.angle.x, it.angle.y, DirectX::XMConvertToRadians(it.angle.z));
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * R * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);
//		constexpr XMFLOAT4 color{ 1.0f,0.3f,0.0f,1.0f };
//		constexpr XMFLOAT4 color{ 1.0f,1.0f,1.0f,1.0f };
//		constexpr XMFLOAT4 color{ 1.0f,0.8f,0.8f,1.0f };
		XMFLOAT4 color = it.color;

		sprSmoke->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}

void ParticleManager::DrawSmokeOfBoss
(
	const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	using namespace DirectX;

	auto Matrix = [](const XMFLOAT4X4 & matrix)
	{
		return XMLoadFloat4x4(&matrix);
	};
	auto Float4x4 = [](const XMMATRIX & M)
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4(&matrix, M);
		return matrix;
	};

	for (auto& it : bossDamageEffects)
	{
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(it.angle.x, it.angle.y, DirectX::XMConvertToRadians(it.angle.z));
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * R * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);
		XMFLOAT4 color = it.color;

		sprSmoke->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}

void ParticleManager::DrawShockWave
(
	const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	using namespace DirectX;

	auto Matrix = [](const XMFLOAT4X4 & matrix)
	{
		return XMLoadFloat4x4(&matrix);
	};
	auto Float4x4 = [](const XMMATRIX & M)
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4(&matrix, M);
		return matrix;
	};

	XMFLOAT4 color{ 0.3f, 0.3f, 0.3f, 1.0f };

	for (auto& it : shockWaveEffects)
	{
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * T;

		XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);

		sprSled->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}

}



/*-------------------------------------------------*/
//	パーティクルを生成する関数
/*-------------------------------------------------*/
void ParticleManager::CreateSledParticle(Donya::Vector3 _pos)
{
	Particle pre(_pos, Particle::Type::SLED_EFFECT);
	sledEffects.emplace_back(pre);
}

void ParticleManager::CreateSmokeOfMissileParticle(Donya::Vector3 _pos)
{
	Particle pre(_pos, Particle::Type::MISSILE_EFFECT);
	missileEffects.emplace_back(pre);
}

void ParticleManager::CreateShockWaveParticle(Donya::Vector3 _pos)
{
	constexpr int MAX_NUM = 70;
	for (int i = 0; i < MAX_NUM; i++)
	{
		Particle pre(_pos, Particle::Type::SHOCKWAVE_EFFECT);
		shockWaveEffects.emplace_back(pre);
	}
}

void ParticleManager::CreateExplosionParticle(Donya::Vector3 _pos, int _loopNum)
{
	if (_loopNum == 0)return;
	Donya::Vector3 emitPos = _pos;
	float arg = 1.0f;
	for (int i = _loopNum; i > 0; i--)
	{
		arg = 1.0f * i / _loopNum;
		Particle pre(emitPos, Particle::Type::MISSILE_EFFECT, true, arg);
		missileEffects.emplace_back(pre);
		emitPos.z += 1.0f;
	}
}

void ParticleManager::ReserveExplosionParticles(Donya::Vector3 _emitPos, int _popNum, int _onceNum)
{
	explosionPos = _emitPos;
	explosionPopNum = _popNum;
	popNumOnce = _onceNum;
	isExplosion = true;
}

void ParticleManager::CreateExplosionLoop()
{
	if (!isExplosion) return;
//	Donya::Vector3 emitPos = _pos;
	Donya::Vector3 emitPos = explosionPos;
	for (int i = 0; i < popNumOnce; i++)
	{
//		CreateExplosionParticle(_pos, explosionPopNum);
		Particle pre(emitPos, Particle::Type::MISSILE_EFFECT, true);
		missileEffects.emplace_back(pre);
		if (--explosionPopNum == 0)
		{
			isExplosion = false;
			break;
		}
		emitPos.z += 1.0f;
	}
}

void ParticleManager::CreateBossDamageLoop(Donya::Vector3 _pos)
{
	if (!isBossSmoke)return;

	int generateInterval = 0;
	switch (damageLevel)
	{
	case LEVEL1:
		generateInterval = 6;	break;
	case LEVEL2:
		generateInterval = 2;	break;
	case LEVEL3:
		generateInterval = 1;	break;
	default:
		break;
	}

	if (timer % generateInterval == 0)
	{
		static bool isEnableRight;
		Donya::Vector3 priPos = _pos;
		if (isEnableRight)
		{
			static bool isPlusZ;
			priPos.x += 40.0f;
			priPos.y += 55.0f;
			if (isPlusZ)
				priPos.z += 1.0f;
			isPlusZ = !isPlusZ;
		}
		else
		{
			static bool isPlusZ2;
			priPos.x -= 40.0f;
			priPos.y += 55.0f;
			if (isPlusZ2)
				priPos.z -= 1.0f;
			isPlusZ2 = !isPlusZ2;
		}
		CreateBossDamageParticle(priPos, isEnableRight);
		isEnableRight = !isEnableRight;

	}
}

void ParticleManager::CreateBossDamageParticle(Donya::Vector3 _pos, bool _isEnableRight)
{
	Particle pre(_pos, Particle::Type::BOSS_DAMAGE_EFFECT, _isEnableRight);
	bossDamageEffects.emplace_back(pre);
}

void ParticleManager::StartBossDamageParticle()
{
	isBossSmoke = true;
	damageLevel = LEVEL1;
}

void ParticleManager::UpdateBossDamageLevel()
{
	switch (damageLevel)
	{
	case LEVEL1:
		damageLevel = LEVEL2;	break;
	case LEVEL2:
		damageLevel = LEVEL3;	break;
	default:
		damageLevel = LEVEL3;	break;
	}
}

/*-------------------------------------------------*/
//	消去するか判定する関数
/*-------------------------------------------------*/
void ParticleManager::JudgeErase()
{
	JudgeEraseSled();
	JudgeEraseSmokeOfMissile();
	JudgeEraseSmokeOfBoss();
	JudgeEraseShockWave();
}

void ParticleManager::JudgeEraseSled()
{
	for (size_t i = 0; i < sledEffects.size(); i++)
	{
		if (sledEffects[i].existanceTime <= 0)
		{
			sledEffects.erase(sledEffects.begin() + i);
			continue;
		}
	}
}

void ParticleManager::JudgeEraseSmokeOfMissile()
{
	for (size_t i = 0; i < missileEffects.size(); i++)
	{
		if (missileEffects[i].existanceTime <= 0)
		{
			missileEffects.erase(missileEffects.begin() + i);
		}
	}
}

void ParticleManager::JudgeEraseSmokeOfBoss()
{
	for (size_t i = 0; i < bossDamageEffects.size(); i++)
	{
		if (bossDamageEffects[i].existanceTime <= 0)
		{
			bossDamageEffects.erase(bossDamageEffects.begin() + i);
		}
	}
}

void ParticleManager::JudgeEraseShockWave()
{
	for (size_t i = 0; i < shockWaveEffects.size(); i++)
	{
		if (shockWaveEffects[i].existanceTime <= 0)
		{
			shockWaveEffects.erase(shockWaveEffects.begin() + i);
		}
	}
}

#if USE_IMGUI

/*-------------------------------------------------*/
//	ImGui関数
/*-------------------------------------------------*/
void ParticleManager::UseImGui()
{
	if (ImGui::BeginIfAllowed())
	{
		if (ImGui::TreeNode(u8"パーティクル"))
		{
			ImGui::SliderFloat("初速X", &Particle::setVelocity.x, -10.0f, 10.0f);
			ImGui::SliderFloat("初速Y", &Particle::setVelocity.y, -10.0f, 10.0f);
			ImGui::SliderFloat("初速Z", &Particle::setVelocity.z, -10.0f, 10.0f);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

#endif // USE_IMGUI

/*-------------------------------------------------*/
//
//	Particle
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	コンストラクタとデストラクタ
/*-------------------------------------------------*/
Particle::Particle()
{
	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	color = Donya::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	type = NONE;
	existanceTime = 0;
}

Particle::Particle(Donya::Vector3 _emitterPos, Type _type, bool _noMove, float _scale)
{
	type = _type;
	existanceTime = 0;
	switch (type)
	{
	case Particle::NONE:				SetNoneElements(_emitterPos);				break;
	case Particle::SLED_EFFECT:			SetSledElements(_emitterPos);				break;
	case Particle::BOSS_DAMAGE_EFFECT:	SetBossElements(_emitterPos, _noMove);				break;
	case Particle::MISSILE_EFFECT:		SetMissileElements(_emitterPos, _noMove, _scale);	break;
	case Particle::SHOCKWAVE_EFFECT:	SetShockWaveElements(_emitterPos);			break;
	default:																		break;
	}
}

Particle::Particle(const Particle& rhs)
{
	*this = rhs;
}

Particle& Particle::operator = (const Particle& rhs)
{
	pos = rhs.pos;
	velocity = rhs.velocity;
	scale = rhs.scale;
	setVelocity = rhs.setVelocity;
	existanceTime = rhs.existanceTime;
	type = rhs.type;
	color = rhs.color;

	return *this;
}

Particle::~Particle()
{

}

/*-------------------------------------------------*/
//	それぞれのパーティクルの初期化
/*-------------------------------------------------*/
void Particle::SetNoneElements(Donya::Vector3 _emitterPos)
{
	velocity = setVelocity;
	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	existanceTime = 0;
	assert("particle type is None");
}

void Particle::SetSledElements(Donya::Vector3 _emitterPos)
{
	//------------------------------
	// Control Emitter Position
	//------------------------------
	pos = _emitterPos;
	int rnd = Donya::Random::GenerateInt(0, 2);
	if (rnd == 0)	pos.x += 5.0f;	//Left side
	else			pos.x -= 5.0f;	// Right side

	float randAdjustPos = Donya::Random::GenerateFloat(-10.0f, 10.0f);
	pos.z += randAdjustPos;

	// Set Scale
	scale = Donya::Vector3(2.0f, 2.0f, 2.0f);
	pos.y += scale.y;

	// Set velocity
	// x = 1.9f		y = 2.133f		z = 0.607f
	Donya::Vector3 randVec = Donya::Vector3(Donya::Random::GenerateFloat(0.4f, 0.9f), Donya::Random::GenerateFloat(0.9f, 1.2f), Donya::Random::GenerateFloat(-1.507f, -2.007f));
	int rand = Donya::Random::GenerateInt(2);
	if (rnd)	randVec.x *= -1;
	setVelocity = randVec;

	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = setVelocity;
	existanceTime = 20;
}

void Particle::SetBossElements(Donya::Vector3 _emitterPos, bool _isEnableRight)
{
	pos = _emitterPos;
	if (_isEnableRight)
	{
		velocity = Donya::Vector3
		(
			Donya::Random::GenerateFloat(0.0f, 2.0f),
			Donya::Random::GenerateFloat(3.0f, 5.0f),
			-10.0f
		);
	}
	else
	{
		velocity = Donya::Vector3
		(
			Donya::Random::GenerateFloat(-2.0f, 0.0f),
			Donya::Random::GenerateFloat(3.0f, 5.0f),
			-10.0f
		);
	}
	scale = Donya::Vector3(50.0f, 50.0f, 50.0f);
//	color = Donya::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	color = Donya::Vector4(0.8f, 0.8f, 0.8f, 1.0f);
	angle = Donya::Vector3(0.0f, 0.0f, Donya::Random::GenerateFloat(0.0f,360.0f));
	existanceTime = 30;
}

void Particle::SetMissileElements(Donya::Vector3 _emitterPos, bool _noMove, float _scale)
{
	if (_noMove)
	{
		if (_scale == 1.0f) scale = Donya::Vector3(100.0f, 100.0f, 100.0f);
		else 				scale = Donya::Vector3(100.0f * _scale, 100.0f * _scale, 100.0f * _scale);
		velocity = Donya::Vector3(0.0f,0.0f,-10.0f);
		color = Donya::Vector4(1.0f, 0.3f, 0.0f, 1.0f);
	}
	else
	{
	velocity = Donya::Vector3(
		Donya::Random::GenerateFloat(-3.0f,3.0f), 
		Donya::Random::GenerateFloat(-3.0f, 3.0f), 
		0.0f//Donya::Random::GenerateFloat(-3.0f, 3.0f)
	);
	color = Donya::Vector4(1.0f, 0.8f, 0.8f, 1.0f);
	scale = Donya::Vector3(50.0f, 50.0f, 50.0f);
	}


	pos = Donya::Vector3(_emitterPos.x, _emitterPos.y + 0.0f, _emitterPos.z + 10.0f);
	angle.x = Donya::Random::GenerateFloat(0.0f, 360.0f);
	angle.y = Donya::Random::GenerateFloat(0.0f, 360.0f);
	angle.z = Donya::Random::GenerateFloat(0.0f, 360.0f);
	if (_noMove)
	{
		existanceTime = 15;
	}
	else
	{
		existanceTime = 15;
	}
}

void Particle::SetShockWaveElements(Donya::Vector3 _emitterPos)
{
	using namespace Donya;
	pos = _emitterPos;
	// 64.0f is Wave width size.
	pos.x += Donya::Random::GenerateFloat(-64.0f, 64.0f);

	velocity = Vector3{ 0.0f,/*10.0f*/Donya::Random::GenerateFloat(3.0f,10.0f),0.0f };
	scale = Vector3{ 2.0f,2.0f,2.0f };
	angle = Vector3{ 0.0f,0.0f,0.0f };
	existanceTime = 30;
}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Particle::UpdateOfSleds()
{
	velocity.y -= GRAVITY;
	pos += velocity;
	--existanceTime;
}

void Particle::UpdateOfMissiles()
{

	// Update Position
	pos += velocity;

	// Update angle
	angle.z += Donya::Random::GenerateFloat(50.0f, 100.0f);

	if (angle.x >= 360) angle.x = 0;
	if (angle.y >= 360)angle.y = 0;
	if (angle.z >= 360)angle.z = 0;
	// Update scale
	scale.x = scale.y = scale.x * existanceTime / 15 ;
	--existanceTime;
}

void Particle::UpdateOfBossDamage()
{

	pos += velocity;
	angle.z -= Donya::Random::GenerateFloat(5.0f, 10.0f);
	if (angle.z >= 360)angle.z = 0;
	scale.x = scale.y = scale.z = 50.0f * (static_cast<float>(existanceTime) / 30.0f);
	--existanceTime;
}

void Particle::UpdateOfShockWave()
{
	constexpr float GRAVITY = -0.98f;

	velocity.y += GRAVITY;

	pos += velocity;
	--existanceTime;
}