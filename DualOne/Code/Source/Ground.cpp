#include "Ground.h"

#include "Donya/Loader.h"
#include "Donya/UseImgui.h"
#include "Donya/Collision.h"
#include "Donya/Random.h"
#include "FilePath.h"

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

void Block::Init(size_t _num)
{
	billBoard.reset();
//	cube.Init();
	if (_num % 2 == 0)
	{
		billBoard = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::GroundTex1));
	}
	else
	{
		billBoard = std::make_unique<Donya::Geometric::TextureBoard>(GetSpritePath(SpriteAttribute::GroundTex2));
	}
	billBoard->Init();
	if (_num == 0)
		pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	else if (_num == 1)
		pos = Donya::Vector3(0.0f, 0.0f, -500.0f);
	else if (_num == 2)
		pos = Donya::Vector3(0.0f, 0.0f, -1000.0f);
	else
		pos = Donya::Vector3(0.0f, 0.0f, -1500.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(100.0f, 1.0f, 2000.0f);
}
/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Block::Update(Donya::Vector3 _playerPos)
{
	Move();
	ApplyLoopToMap(_playerPos);
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
	//�t�s��̍쐬
#if 1
	DirectX::XMFLOAT4X4		view2 = matView;
	DirectX::XMMATRIX		inv_view2;
	view2._14 = view2._24 = view2._34 = 0.0f;
	view2._41 = 0.0f; view2._42 = 0.0f; 							//	�ʒu��񂾂����폜
	view2._43 = 0.0f; view2._44 = 1.0f;								//	
	inv_view2 = DirectX::XMLoadFloat4x4(&view2);					//	Matrix�^�֍ĕϊ�
	inv_view2 = DirectX::XMMatrixInverse(nullptr, inv_view2);		//	view�s��̋t�s��쐬

	//�r���[�s��A���e�s������������s��̍쐬
	DirectX::XMMATRIX	VPSynthesisMatrix = Matrix(matView) * Matrix(matProjection);

	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * T;

	// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
	XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;
#else
	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * T;

	XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);


#endif
	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	billBoard->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
}

/*-------------------------------------------------*/
//	�ړ��֐�
/*-------------------------------------------------*/
void Block::Move()
{
	pos += velocity;
}

/*-------------------------------------------------*/
//	�ړ��֐�
/*-------------------------------------------------*/
void Block::ApplyLoopToMap(Donya::Vector3 _playerPos)
{
	if (pos.z >= _playerPos.z + 1000)
	{
		pos.z = _playerPos.z -1000;
	}
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
	for (size_t i = 0;i<blocks.size();i++)
	{
		blocks[i].Init(i);
	}
}

void Ground::Uninit()
{

}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Ground::Update(Donya::Vector3 _playerPos)
{
	for (auto& it : blocks)
	{
		it.Update(_playerPos);
	}
	for (auto& it : trees)
	{
		it.Update();
	}

	if (++timer >= 15)
	{
		timer = 0;
		Donya::Vector3 treePos;
		static bool generateDir;
		generateDir = !generateDir;
		if (generateDir)	treePos.x = 70.0f;
		else				treePos.x = -70.0f;
		treePos.y = 0.0f;
		treePos.z = _playerPos.z - 200.0f;
		CreateTree(treePos);
	}

	EraseDeadTree(_playerPos);
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
	for (auto& it : blocks)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
	for (auto& it : trees)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
}

/*-------------------------------------------------*/
//	�؂̐����֐�
/*-------------------------------------------------*/
void Ground::CreateTree(Donya::Vector3 _pos)
{
	trees.push_back({});
	trees.back().Init(_pos);
}

/*-------------------------------------------------*/
//	�؂̍폜�֐�
/*-------------------------------------------------*/
void Ground::EraseDeadTree(Donya::Vector3 _playerPos)
{
	auto eraseItr = std::remove_if
	(
		trees.begin(), trees.end(),
		[&](Tree & element)
		{
			return element.ShouldErase(_playerPos);
		}
	);
	trees.erase(eraseItr, trees.end());
}


/*-------------------------------------------------*/
//
//	Tree
//
/*-------------------------------------------------*/
std::shared_ptr<Donya::StaticMesh> Tree::pModel{ nullptr };
/*-------------------------------------------------*/
//	�������֐�
/*-------------------------------------------------*/
Tree::Tree()
{
	pos = Donya::Vector3(0.0f, 0.0f, 0.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);
	scale = Donya::Vector3(0.0f, 0.0f, 0.0f);
	LoadModel();
}

Tree::~Tree()
{

}

/*-------------------------------------------------*/
//	�������֐�
/*-------------------------------------------------*/
void Tree::Init(Donya::Vector3 _pos)
{
	pos = _pos;
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(1.8f, 1.8f, 1.8f);
	LoadModel();
}

/*-------------------------------------------------*/
//	�X�V�֐�
/*-------------------------------------------------*/
void Tree::Update()
{

}

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
