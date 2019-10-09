#include "Effect.h"
#include "FilePath.h"

/*-------------------------------------------------*/
//
//	ParticleManager
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	�������֐�
/*-------------------------------------------------*/
void ParticleManager::Init()
{
	LoadSprite();
	timer = 0;
}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void ParticleManager::Update()
{
	for (auto& it : sledEffect)
	{
		it.Update();
	}
}

/*-------------------------------------------------*/
//	�`��֐�
/*-------------------------------------------------*/
void ParticleManager::Draw(const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	DrawSled(matView, matProjection, lightDirection, cameraPosition, isEnableFill);
}

/*-------------------------------------------------*/
//	�I���֐�
/*-------------------------------------------------*/
void ParticleManager::Uninit()
{

}

/*-------------------------------------------------*/
//	Sprite�̃��[�h�֐�
/*-------------------------------------------------*/
void ParticleManager::LoadSprite()
{
	sprSled = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::SledEffect));
}

/*-------------------------------------------------*/
//	Sled�̕`��֐�
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
	view2._41 = 0.0f; view2._42 = 0.0f; 							//	�ʒu��񂾂����폜
	view2._43 = 0.0f; view2._44 = 1.0f;								//	
	inv_view2 = DirectX::XMLoadFloat4x4(&view2);					//	Matrix�^�֍ĕϊ�
	inv_view2 = DirectX::XMMatrixInverse(nullptr, inv_view2);		//	view�s��̋t�s��쐬

	//�r���[�s��A���e�s������������s��̍쐬
	DirectX::XMMATRIX	VPSynthesisMatrix = Matrix(matView) * Matrix(matProjection);

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	for (auto& it : sledEffect)
	{
		XMMATRIX S = DirectX::XMMatrixScaling(it.scale.x, it.scale.y, it.scale.z);
		//	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
		XMMATRIX T = DirectX::XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		XMMATRIX W = S * T;

		// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
		XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;

		sprSled->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}

}
/*-------------------------------------------------*/
//	�\���̃p�[�e�B�N���𐶐�����֐�
/*-------------------------------------------------*/
void ParticleManager::CreateSledParticle(Donya::Vector3 _pos)
{
	Particle pre(_pos, Particle::Type::SLED_EFFECT);
	sledEffect.emplace_back(pre);
}

/*-------------------------------------------------*/
//	�������邩���肷��֐�
/*-------------------------------------------------*/
void ParticleManager::JudgeEraseSled()
{
	for (size_t i = 0; i < sledEffect.size(); i++)
	{
		if (sledEffect[i].existanceTime <= 0)
		{
//			sledEffect.erase(sledEffect.begin() + i);
		}
	}
}

/*-------------------------------------------------*/
//
//	Particle
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	�R���X�g���N�^�ƃf�X�g���N�^
/*-------------------------------------------------*/
Particle::Particle()
{
	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
	type = NONE;
	existanceTime = 0;
}

Particle::Particle(Donya::Vector3 _emitterPos, Type _type)
{
	pos = _emitterPos;
	type = _type;
	scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
	switch (type)
	{
	case Particle::NONE:
		assert("particle type is None");
		velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
		existanceTime = 0;
		break;
	case Particle::SLED_EFFECT:
		velocity = Donya::Vector3(3.0f, 5.0f, 0.0f);
		existanceTime = 30;
		break;
	case Particle::BOSS_EFFECT:
		velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
		existanceTime = 0;
		break;
	case Particle::MISSLE_EFFECT:
		velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
		existanceTime = 0;
		break;
	default:
		break;
	}
}

Particle::~Particle()
{

}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Particle::Update()
{
	velocity.y -= GRAVITY;
	pos += velocity;
	--existanceTime;
}

