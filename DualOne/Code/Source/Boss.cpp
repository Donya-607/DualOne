#include "Boss.h"

/*-------------------------------------------------*/
//
//	Boss
//
/*-------------------------------------------------*/
/*-------------------------------------------------*/
//	èâä˙âªä÷êî
/*-------------------------------------------------*/
void Boss::Init()
{
	pos = Donya::Vector3(0.0f,20.0f, 150.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(60.0f, 40.0f, 100.0f);
	cube.Init();
}

/*-------------------------------------------------*/
//	çXêVä÷êî
/*-------------------------------------------------*/
void Boss::Update()
{
	Move();
}

/*-------------------------------------------------*/
//	ï`âÊä÷êî
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
}

/*-------------------------------------------------*/
//	èIóπä÷êî
/*-------------------------------------------------*/
void Boss::Uninit()
{

}



/*-------------------------------------------------*/
//	à⁄ìÆä÷êî
/*-------------------------------------------------*/
void Boss::Move()
{
	pos += velocity;
}