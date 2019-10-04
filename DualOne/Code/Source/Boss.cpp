#include "Boss.h"

/*-------------------------------------------------*/
//
//	Boss
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	�������֐�
/*-------------------------------------------------*/
void Boss::Init()
{
	pos = Donya::Vector3(0.0f,20.0f, 150.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(60.0f, 40.0f, 100.0f);
	cube.Init();

	for (auto& it : missle)
	{
		it.Init();
	}
}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Boss::Update()
{
	Move();

	for (auto& it : missle)
	{
		it.Update(pos);
	}

}

/*-------------------------------------------------*/
//	�`��֐�
/*-------------------------------------------------*/
void Boss::Draw(
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

	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = DirectX::XMMatrixIdentity();
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * R * T;
	XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);
	constexpr XMFLOAT4 color{ 1.0f, 0.0f, 0.0f, 1.0f };

	cube.Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	for (auto& it : missle)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
}

/*-------------------------------------------------*/
//	�I���֐�
/*-------------------------------------------------*/
void Boss::Uninit()
{
	for (auto& it : missle)
	{
		it.Uninit();
	}
}



/*-------------------------------------------------*/
//	�ړ��֐�
/*-------------------------------------------------*/
void Boss::Move()
{
	pos += velocity;
}




/*-------------------------------------------------*/
//
//	Missle
//
/*-------------------------------------------------*/
/*-----------------------------------------*/
//	�������֐�
/*-----------------------------------------*/
void Missle::Init()
{
	sphere.Init();

	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
	state = INITIALIZE;
}

/*-----------------------------------------*/
//	�X�V�֐�
/*-----------------------------------------*/
void Missle::Update(Donya::Vector3 bossPos)
{
	Move(bossPos);
}


/*-----------------------------------------*/
//	�`��֐�
/*-----------------------------------------*/
void Missle::Draw(
	const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{

}

/*-----------------------------------------*/
//	�I���֐�
/*-----------------------------------------*/
void Missle::Uninit()
{

}


/*-----------------------------------------*/
//	�X�V�֐�
/*-----------------------------------------*/
void Missle::Move(Donya::Vector3 bossPos)
{
	switch (state)
	{
	case Missle::NOT_ENABLE:

		break;
	case Missle::INITIALIZE:

//		break;	�Ӑ}�I�ȃR�����g�A�E�g
	case Missle::PREP_MOVE:

		break;
	case Missle::PREP_STOP:

		break;
	case Missle::ATTACK_MOVE:

		break;
	case Missle::END:

		break;
	default:
		break;
	}
	pos += velocity;
}