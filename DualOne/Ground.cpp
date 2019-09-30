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
	for (auto& it : cube)
	{
		it.Init();
	}
	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	size = Donya::Vector3(1.0f, 1.0f, 1.0f);
}

Block::~Block()
{

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
void Block::Draw()
{

	for (auto& it : cube)
	{
//		it.Render();
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

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Ground::Update()
{
	for (auto& it : block)
	{
		it.Update();
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
		it.Draw();
	}
}

/*-------------------------------------------------*/
//	��񐶐��֐�
/*-------------------------------------------------*/
void Ground::Create()
{
	Block pre;
	block.emplace_back(pre);
}