#include "Ground.h"

/*-------------------------------------------------*/
//
//	Block
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	コンストラクタとデストラクタ
/*-------------------------------------------------*/
Block::Block()
{
}

Block::~Block()
{

}

void Block::Init()
{
	cube.Init();
	pos = Donya::Vector3(0.0f, 0.0f, -10.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(48.5f, 1.0f, 1000.0f);
}
/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Block::Update()
{
	Move();
}

/*-------------------------------------------------*/
//	描画関数
/*-------------------------------------------------*/
void Block::Draw(
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

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	cube.Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
}

/*-------------------------------------------------*/
//	移動関数
/*-------------------------------------------------*/
void Block::Move()
{
	pos += velocity;
}



/*-------------------------------------------------*/
//
//	Ground
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	コンストラクタとデストラクタ
/*-------------------------------------------------*/
Ground::Ground()
{

}

Ground::~Ground()
{

}

void Ground::Init()
{
	timer = 0;
	for (auto& it : block)
	{
		it.Init();
	}
	Create();
}

void Ground::Uninit()
{

}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Ground::Update()
{
	for (auto& it : block)
	{
		it.Update();
	}

	if (++timer >= 10)
	{
		timer = 0;
		Create();
	}
}

/*-------------------------------------------------*/
//	描画関数
/*-------------------------------------------------*/
void Ground::Draw(
	const DirectX::XMFLOAT4X4& matView,
	const DirectX::XMFLOAT4X4& matProjection,
	const DirectX::XMFLOAT4& lightDirection,
	const DirectX::XMFLOAT4& cameraPosition,
	bool isEnableFill
)
{
	for (auto& it : block)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
}

/*-------------------------------------------------*/
//	一列生成関数
/*-------------------------------------------------*/
void Ground::Create()
{
	Block pre;
	pre.Init();
	block.emplace_back(pre);
}