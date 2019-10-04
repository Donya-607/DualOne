#include "Boss.h"

/*-------------------------------------------------*/
//
//	Boss
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	初期化関数
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
//	更新関数
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
//	描画関数
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
//	終了関数
/*-------------------------------------------------*/
void Boss::Uninit()
{
	for (auto& it : missle)
	{
		it.Uninit();
	}
}



/*-------------------------------------------------*/
//	移動関数
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
//	初期化関数
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
//	更新関数
/*-----------------------------------------*/
void Missle::Update(Donya::Vector3 bossPos)
{
	Move(bossPos);
}


/*-----------------------------------------*/
//	描画関数
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
//	終了関数
/*-----------------------------------------*/
void Missle::Uninit()
{

}


/*-----------------------------------------*/
//	更新関数
/*-----------------------------------------*/
void Missle::Move(Donya::Vector3 bossPos)
{
	switch (state)
	{
	case Missle::NOT_ENABLE:

		break;
	case Missle::INITIALIZE:

//		break;	意図的なコメントアウト
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