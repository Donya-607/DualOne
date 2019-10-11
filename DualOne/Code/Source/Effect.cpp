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
}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void ParticleManager::Update(ParticleEmitterPosition _arg)
{
	for (int i = 0; i < 3; i++)
	{
		CreateSledParticle(_arg.playerPos);
	}
	JudgeEraseSled();
	JudgeEraseSmokeOfMissle();

	for (auto& it : sledEffects)
	{
		it.UpdateOfSleds();
	}
	for (auto& it : missleEffects)
	{
		it.UpdateOfMissles();
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
	DrawSmokeOfMissle(matView, matProjection, lightDirection, cameraPosition, isEnableFill);
	Donya::Blend::Set(Donya::Blend::Mode::ALPHA);
}

/*-------------------------------------------------*/
//	終了関数
/*-------------------------------------------------*/
void ParticleManager::Uninit()
{

}

/*-------------------------------------------------*/
//	Spriteのロード関数
/*-------------------------------------------------*/
void ParticleManager::LoadSprite()
{
	sprSled = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::SledEffect));
	sprSled->Init();

	sprSmoke = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::SmokeEffect));
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
		//	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;

		sprSled->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}

void ParticleManager::DrawSmokeOfMissle
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

//	DirectX::XMFLOAT4X4		view2 = matView;
//	DirectX::XMMATRIX		inv_view2;
//	view2._14 = view2._24 = view2._34 = 0.0f;
//	view2._41 = 0.0f; view2._42 = 0.0f; 							//	位置情報だけを削除
//	view2._43 = 0.0f; view2._44 = 1.0f;								//	
//	inv_view2 = DirectX::XMLoadFloat4x4(&view2);					//	Matrix型へ再変換
//	inv_view2 = DirectX::XMMatrixInverse(nullptr, inv_view2);		//	view行列の逆行列作成
//
//	//ビュー行列、投影行列を合成した行列の作成
//	DirectX::XMMATRIX	VPSynthesisMatrix = Matrix(matView) * Matrix(matProjection);
//
//	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
//
//	for (auto& it : missleEffects)
//	{
//		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
//		XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, it.angle);
//		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
//		XMMATRIX W = S * T;
//
//		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
//		XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;
//
//		sprSmoke->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
//	}
	for (auto& it : missleEffects)
	{
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(it.angle.x, it.angle.y, DirectX::XMConvertToRadians(it.angle.z));
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * R * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);
		constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

		sprSmoke->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}


/*-------------------------------------------------*/
//	ソリのパーティクルを生成する関数
/*-------------------------------------------------*/
void ParticleManager::CreateSledParticle(Donya::Vector3 _pos)
{
	Particle pre(_pos, Particle::Type::SLED_EFFECT);
	sledEffects.emplace_back(pre);
}

void ParticleManager::CreateSmokeOfMissleParticle(Donya::Vector3 _pos)
{
	Particle pre(_pos, Particle::Type::MISSLE_EFFECT);
	missleEffects.emplace_back(pre);
}

/*-------------------------------------------------*/
//	消去するか判定する関数
/*-------------------------------------------------*/
void ParticleManager::JudgeEraseSled()
{
	for (size_t i = 0; i < sledEffects.size(); i++)
	{
		if (sledEffects[i].existanceTime <= 0)
		{
			sledEffects.erase(sledEffects.begin() + i);
			continue;
		}
//		else if (sledEffects[i].pos.y - sledEffects[i].scale.y <= 0)
//		{
//			sledEffects.erase(sledEffects.begin() + i);
//		}
	}
}

void ParticleManager::JudgeEraseSmokeOfMissle()
{
	for (size_t i = 0; i < missleEffects.size(); i++)
	{
		if (missleEffects[i].existanceTime <= 0)
		{
			missleEffects.erase(missleEffects.begin() + i);
		}
	}
}

/*-------------------------------------------------*/
//	消去するか判定する関数
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
	type = NONE;
	existanceTime = 0;
}

Particle::Particle(Donya::Vector3 _emitterPos, Type _type)
{
	type = _type;
	switch (type)
	{
	case Particle::NONE:			SetNoneElements(_emitterPos);	break;
	case Particle::SLED_EFFECT:		SetSledElements(_emitterPos);	break;
	case Particle::BOSS_EFFECT:		SetBossElements(_emitterPos);	break;
	case Particle::MISSLE_EFFECT:	SetMissleElements(_emitterPos); break;
	default:														break;
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
	startVelocity = rhs.startVelocity;
	scale = rhs.scale;
	setVelocity = rhs.setVelocity;
	existanceTime = rhs.existanceTime;
	type = rhs.type;

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
	startVelocity = velocity;
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
	if (rnd == 0)
	{
		pos.x += 5.0f;	//Left side
	}
	else
	{
		pos.x -= 5.0f;	// Right side
	}

	float randAdjustPos = Donya::Random::GenerateFloat(-10.0f, 10.0f);
	pos.z += randAdjustPos;

	// Set Scale
	scale = Donya::Vector3(2.0f, 2.0f, 2.0f);
	pos.y += scale.y;

	// Set velocity
	// x = 1.9f		y = 2.133f		z = 0.607f
	Donya::Vector3 randVec = Donya::Vector3(Donya::Random::GenerateFloat(0.4f, 0.9f), Donya::Random::GenerateFloat(0.9f, 1.2f), Donya::Random::GenerateFloat(-1.507f, -2.007f));
	int rand = Donya::Random::GenerateInt(2);
	if (rnd)
	{
		randVec.x *= -1;
	}
	setVelocity = randVec;

	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = setVelocity;
	startVelocity = velocity;
	existanceTime = 20;
}

void Particle::SetBossElements(Donya::Vector3 _emitterPos)
{
	velocity = setVelocity;
	startVelocity = velocity;
	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	existanceTime = 0;
}

void Particle::SetMissleElements(Donya::Vector3 _emitterPos)
{
	velocity = Donya::Vector3(
		Donya::Random::GenerateFloat(-3.0f,3.0f), 
		Donya::Random::GenerateFloat(-3.0f, 3.0f), 
		0.0f//Donya::Random::GenerateFloat(-3.0f, 3.0f)
	);
	scale = Donya::Vector3(50.0f, 50.0f, 50.0f);
	pos = Donya::Vector3(_emitterPos.x, _emitterPos.y + 50.0f, _emitterPos.z + 100.0f);
	angle.x = Donya::Random::GenerateFloat(0.0f, 360.0f);
	angle.y = Donya::Random::GenerateFloat(0.0f, 360.0f);
	angle.z = Donya::Random::GenerateFloat(0.0f, 360.0f);
	existanceTime = 15;
}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Particle::UpdateOfSleds()
{
	velocity.y -= GRAVITY;
//	velocity.x -= (startVelocity.x / existanceTime);
	pos += velocity;
	--existanceTime;
}

void Particle::UpdateOfMissles()
{
	--existanceTime;

	// Update Position
	pos += velocity;

	// Update angle
//	angle.x += Donya::Random::GenerateFloat(0.0f, 1.0f);
//	angle.y += Donya::Random::GenerateFloat(0.0f, 1.0f);
	angle.z += Donya::Random::GenerateFloat(0.0f, 10.0f);

	if (angle.x >= 360) angle.x = 0;
	if (angle.y >= 360)angle.y = 0;
	if (angle.z >= 360)angle.z = 0;
	// Update scale
	scale.x = scale.y = scale.x * existanceTime / 15;
}
