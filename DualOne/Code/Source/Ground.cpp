#include "Ground.h"

#include "Donya/Loader.h"
#include "Donya/UseImgui.h"

#include "FilePath.h"


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
	cube.Init();
	pos = Donya::Vector3(0.0f, 0.0f, -10.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(48.5f, 1.0f, 1000.0f);
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
	const DirectX::XMFLOAT4X4 & matView,
	const DirectX::XMFLOAT4X4 & matProjection,
	const DirectX::XMFLOAT4 & lightDirection,
	const DirectX::XMFLOAT4 & cameraPosition,
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
	timer = 0;
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
	CreateBlock();
	CreateTree(Donya::Vector3(0.0f, 0.0f, 0.0f));
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
	for (auto& it : trees)
	{
		it.Update();
	}

	if (++timer >= 10)
	{
		timer = 0;
		CreateBlock();
		CreateTree(Donya::Vector3(0.0f, 0.0f, 0.0f));
	}
}

/*-------------------------------------------------*/
//	�`��֐�
/*-------------------------------------------------*/
void Ground::Draw(
	const DirectX::XMFLOAT4X4 & matView,
	const DirectX::XMFLOAT4X4 & matProjection,
	const DirectX::XMFLOAT4 & lightDirection,
	const DirectX::XMFLOAT4 & cameraPosition,
	bool isEnableFill
)
{
	for (auto& it : block)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
	for (auto& it : trees)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
}

/*-------------------------------------------------*/
//	��񐶐��֐�
/*-------------------------------------------------*/
void Ground::CreateBlock()
{
	Block pre;
	pre.Init();
	block.emplace_back(pre);
}

/*-------------------------------------------------*/
//	�؂̐����֐�
/*-------------------------------------------------*/
void Ground::CreateTree(Donya::Vector3 _pos)
{
	Tree pre(_pos);
	trees.emplace_back(pre);
}



/*-------------------------------------------------*/
//
//	Tree
//
/*-------------------------------------------------*/

std::shared_ptr<Donya::StaticMesh> Tree::pModel{ nullptr };
/*-------------------------------------------------*/
//	�R���X�g���N�^�ƃf�X�g���N�^
/*-------------------------------------------------*/
Tree::Tree()
{
	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(0.0f, 0.0f, 0.0f);
	LoadModel();
}

Tree::Tree(Donya::Vector3 _pos)
{
	pos = _pos;
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
	LoadModel();
}

Tree::~Tree()
{

}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Tree::Update()
{
	//	UseImGui();
	//	pos += velocity;
}
#include "Donya/Collision.h"
/*-------------------------------------------------*/
//	�`��֐�
/*-------------------------------------------------*/
void Tree::Draw
(
	const DirectX::XMFLOAT4X4 & matView,
	const DirectX::XMFLOAT4X4 & matProjection,
	const DirectX::XMFLOAT4 & lightDirection,
	const DirectX::XMFLOAT4 & cameraPosition,
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

	pModel->Render(Float4x4(WVP), Float4x4(W), lightDirection, color, cameraPosition);

	//if (Common::IsShowCollision())
	{
		AABB wsBody
		{
			{0,0,0},
			{scale.x, scale.y, scale.z},
			true
		};
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		XMMATRIX colS = XMMatrixScaling(wsBody.size.x, wsBody.size.y, wsBody.size.z);
		XMMATRIX colT = XMMatrixTranslation(wsBody.pos.x, wsBody.pos.y, wsBody.pos.z);
		XMMATRIX colW = colS * R * colT;

		XMMATRIX colWVP = colW * Matrix(matView) * Matrix(matProjection);

		constexpr XMFLOAT4 colColor{ 1.0f, 0.3f, 1.0f, 0.5f };

		auto InitializedCube = []()
		{
			Donya::Geometric::Cube cube{};
			cube.Init();
			return cube;
		};
		static Donya::Geometric::Cube cube = InitializedCube();
		cube.Render
		(
			Float4x4(colWVP),
			Float4x4(colW),
			lightDirection,
			colColor
		);

	}
}

/*-------------------------------------------------*/
//	�ړ��֐�
/*-------------------------------------------------*/
void Tree::Move()
{

}

/*-------------------------------------------------*/
//	�ړ��֐�
/*-------------------------------------------------*/
void Tree::LoadModel()
{
	static bool wasLoaded = false;
	if (wasLoaded) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load(GetModelPath(ModelAttribute::Tree), nullptr);

	_ASSERT_EXPR(result, L"Failed : Load boss's missile model.");

	pModel = Donya::StaticMesh::Create(loader);

	if (!pModel)
	{
		_ASSERT_EXPR(0, L"Failed : Load boss's missile model.");
		exit(-1);
	}

	wasLoaded = true;
}

void Tree::UseImGui()
{
	if (ImGui::BeginIfAllowed())
	{
		ImGui::SliderFloat(u8"�؂̍��WX", &pos.x, -100.0f, 100.f);
		ImGui::SliderFloat(u8"�؂̍��WY", &pos.y, -100.0f, 100.f);
		ImGui::SliderFloat(u8"�؂̍��WZ", &pos.z, -100.0f, 100.f);
		ImGui::SliderFloat(u8"�؂�velocity X", &velocity.x, -100.0f, 100.f);
		ImGui::SliderFloat(u8"�؂�velocity Y", &velocity.y, -100.0f, 100.f);
		ImGui::SliderFloat(u8"�؂�velocity Z", &velocity.z, -100.0f, 100.f);
		ImGui::End();
	}
}
