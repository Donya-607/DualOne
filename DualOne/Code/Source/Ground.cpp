#include "Ground.h"

/*-------------------------------------------------*/
//
//	Block
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	�R���X�g���N�^�ƃf�X�g���N�^
/*-------------------------------------------------*/
Block::Block()
{
}

Block::~Block()
{

}

void Block::Init()
{
	for (auto& it : cube)
	{
		it.Init();
	}
	pos = Donya::Vector3(0.0f, 0.0f, -10.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.1f);
	size = Donya::Vector3(1.0f, 1.0f, 1.0f);
}
/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Block::Update()
{
	Move();
}

/*-------------------------------------------------*/
//	�`��֐�
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

	XMMATRIX S = DirectX::XMMatrixIdentity();
	XMMATRIX R = DirectX::XMMatrixIdentity();
	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	for ( int i = 0; i < static_cast<int>( cube.size() )/*7*/; i++ )
	{
		XMMATRIX T = DirectX::XMMatrixTranslation(static_cast<float>(i-3), pos.y, pos.z);
		XMMATRIX W = S * R * T;
		XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);

		cube[i].Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
	}
}

/*-------------------------------------------------*/
//	�ړ��֐�
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
//	�R���X�g���N�^�ƃf�X�g���N�^
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
//	�X�V�֐�
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
//	�`��֐�
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
//	��񐶐��֐�
/*-------------------------------------------------*/
void Ground::Create()
{
	Block pre;
	pre.Init();
	block.emplace_back(pre);
}