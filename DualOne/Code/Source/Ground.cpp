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
//	コンストラクタとデストラクタ
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
		pos = Donya::Vector3(0.0f, 0.0f, 5000.0f);
	else if (_num == 1)
		pos = Donya::Vector3(0.0f, 0.0f, -0.0f);
	else if (_num == 2)
		pos = Donya::Vector3(0.0f, 0.0f, -5000.0f);
	else
		pos = Donya::Vector3(0.0f, 0.0f, -10000.0f);
	velocity = Donya::Vector3(0.0f, 0.0f, 0.0f);

#if 0
	scale = Donya::Vector3(100.0f, 1.0f, 2000.0f);
#else
	scale = Donya::Vector3(3000.0f, 10100.0f, 0.1f);

	if (_num % 2 == 0)
		pos.y += -1.0f;
#endif
}
/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Block::Update(Donya::Vector3 _playerPos)
{
	ApplyLoopToMap(_playerPos);
	Move();
}

/*-------------------------------------------------*/
//	描画関数
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
	//逆行列の作成
#if 0
	DirectX::XMFLOAT4X4		view2 = matView;
	DirectX::XMMATRIX		inv_view2;
	view2._14 = view2._24 = view2._34 = 0.0f;
	view2._41 = 0.0f; view2._42 = 0.0f; 							//	位置情報だけを削除
	view2._43 = 0.0f; view2._44 = 1.0f;								//	
	inv_view2 = DirectX::XMLoadFloat4x4(&view2);					//	Matrix型へ再変換
	inv_view2 = DirectX::XMMatrixInverse(nullptr, inv_view2);		//	view行列の逆行列作成

	//ビュー行列、投影行列を合成した行列の作成
	DirectX::XMMATRIX	VPSynthesisMatrix = Matrix(matView) * Matrix(matProjection);

	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * T;

	// WVP  =  WorldMatrix * InverceViewMatrix * SynthesisMatrix of View and Projection
	XMMATRIX WVP = inv_view2 * W * VPSynthesisMatrix;
#else
	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw( DirectX::XMConvertToRadians(90.0f), 0.0f, 0.0f);
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * R * T;

	XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);


#endif
	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	billBoard->Render(Float4x4(WVP), Float4x4(W), lightDirection, color);
}

/*-------------------------------------------------*/
//	移動関数
/*-------------------------------------------------*/
void Block::Move()
{
	pos += velocity;
}

/*-------------------------------------------------*/
//	移動関数
/*-------------------------------------------------*/
void Block::ApplyLoopToMap(Donya::Vector3 _playerPos)
{
	if (pos.z >= _playerPos.z + 10000)
	{
		pos.z = _playerPos.z -10000;
	}
}

/*-------------------------------------------------*/
//
//	Ground
//
/*-------------------------------------------------*/
Donya::Vector3 Ground::treeAngle{ 30.0f, 0.0f, 0.0f };
Donya::Vector3 Ground::treePos{ 0.0f, -4.0f, 0.0f };
int Ground::CREATE_TREE;

/*-------------------------------------------------*/
//	コンストラクタとデストラクタ
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

	for (size_t i = 0; i < walls.size(); i++)
	{
		walls[i].Init(i);
	}
	CREATE_TREE = 15;
	treePos.y = 0.0f;

}

void Ground::Uninit()
{

}

/*-------------------------------------------------*/
//	更新関数
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
	for (auto& it : walls)
	{
		it.Update(_playerPos);
	}

	if (++timer >= CREATE_TREE)
	{
		timer = 0;
		static bool generateDir;
		generateDir = !generateDir;
		if (generateDir)	treePos.x = 70.0f;
		else				treePos.x = -70.0f;
//		treePos.y = 0.0f;
		treePos.z = _playerPos.z - 200.0f;
		CreateTree(treePos);
	}

	EraseDeadTree(_playerPos);

#if USE_IMGUI

	UseImGui();

#endif // USE_IMGUI
}

/*-------------------------------------------------*/
//	描画関数
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
	for (auto& it : walls)
	{
		it.Draw(matView, matProjection, lightDirection, cameraPosition);
	}
}

/*-------------------------------------------------*/
//	木の生成関数
/*-------------------------------------------------*/
void Ground::CreateTree(Donya::Vector3 _pos)
{
	trees.push_back({});
	trees.back().Init(_pos);
}

/*-------------------------------------------------*/
//	木の削除関数
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

#if USE_IMGUI
void Ground::UseImGui()
{
	if (ImGui::BeginIfAllowed())
	{
		if (ImGui::TreeNode(u8"block"))
		{
			Donya::Vector3 math;
			math = blocks[0].GetPos();
			ImGui::SliderFloat(u8"座標 : 0", &math.z, 0.0f, 180.0f);
			math = blocks[1].GetPos();
			ImGui::SliderFloat(u8"座標 : 1", &math.z, 0.0f, 180.0f);
			math = blocks[2].GetPos();
			ImGui::SliderFloat(u8"座標 : 2", &math.z, 0.0f, 180.0f);
			math = blocks[3].GetPos();
			ImGui::SliderFloat(u8"座標 : 3", &math.z, 0.0f, 180.0f);

			ImGui::TreePop();
		}
		if (ImGui::TreeNode(u8"tree"))
		{
			ImGui::SliderFloat(u8"角度X", &treeAngle.x, 0.0f, 180.0f);
			ImGui::SliderFloat(u8"角度Y", &treeAngle.y, 0.0f, 180.0f);
			ImGui::SliderFloat(u8"角度Z", &treeAngle.z, -180.0f, 180.0f);
			ImGui::SliderFloat(u8"Y座標", &treePos.y, -50.0f, 50.0f);
			ImGui::SliderInt(u8"生成間隔", &CREATE_TREE, 1, 60);

			ImGui::TreePop();
		}

		ImGui::End();
	}
}
#endif


/*-------------------------------------------------*/
//
//	Tree
//
/*-------------------------------------------------*/
std::shared_ptr<Donya::StaticMesh> Tree::pModel{ nullptr };
/*-------------------------------------------------*/
//	初期化関数
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
//	初期化関数
/*-------------------------------------------------*/
void Tree::Init(Donya::Vector3 _pos)
{
	pos = _pos;
	velocity = Donya::Vector3(0.0f, 0.0f, -4.0f);
	scale = Donya::Vector3(1.8f, 1.8f, 1.8f);
	angle = Donya::Vector3(0.0f, 0.0f, 0.0f);
	LoadModel();
}

/*-------------------------------------------------*/
//	更新関数
/*-------------------------------------------------*/
void Tree::Update()
{

}

/*-------------------------------------------------*/
//	描画関数
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
	XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(Ground::treeAngle.x), DirectX::XMConvertToRadians(Ground::treeAngle.y), DirectX::XMConvertToRadians(Ground::treeAngle.z));
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * R * T;
	XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	pModel->Render(Float4x4(WVP), Float4x4(W), lightDirection, color, cameraPosition);
}

/*-------------------------------------------------*/
//	モデルのロード関数
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

/*-------------------------------------------------*/
//
//	Wall
//
/*-------------------------------------------------*/
std::shared_ptr<Donya::StaticMesh> Wall::pModel;
/*-------------------------------------------------*/
//	初期化関数
/*-------------------------------------------------*/
void Wall::Init(int _num)
{
	LoadModel();
	if (_num == 0)
	{
		pos = Donya::Vector3(200.0f, 0.0f, -500.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = false;
	}
	else if(_num == 1)
	{
		pos = Donya::Vector3(-200.0f, 0.0f, 0.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = true;
	}
	else if (_num == 2)
	{
		pos = Donya::Vector3(200.0f, 0.0f, 500.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = false;
	}
	else if(_num == 3)
	{
		pos = Donya::Vector3(-200.0f, 0.0f, 1000.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = true;
	}
	else if (_num == 4)
	{
		pos = Donya::Vector3(200.0f, 0.0f, 1500.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = false;
	}
	else if (_num == 5)
	{
		pos = Donya::Vector3(-200.0f, 0.0f, 2000.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = true;
	}
	else if (_num == 6)
	{
		pos = Donya::Vector3(200.0f, 0.0f, 2500.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = false;
	}
	else if (_num == 7)
	{
		pos = Donya::Vector3(-200.0f, 0.0f, 3000.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = true;
	}
	else if (_num == 8)
	{
		pos = Donya::Vector3(200.0f, 0.0f, 3500.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = false;
	}
	else if (_num == 9)
	{
		pos = Donya::Vector3(-200.0f, 0.0f, 4000.0f);
		scale = Donya::Vector3(1.0f, 1.0f, 1.0f);
		isRightWall = true;
	}
}

void Wall::Update(Donya::Vector3 _playerPos)
{
	ApplyLoopToMap(_playerPos);
}

void Wall::Draw(
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
	XMMATRIX R;
	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	if (isRightWall)
		R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(0.0f), 0.0f);
	else
		R = DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(180.0f), 0.0f);
	XMMATRIX T = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX W = S * R * T;
	XMMATRIX WVP = W * Matrix(matView) * Matrix(matProjection);

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	pModel->Render(Float4x4(WVP), Float4x4(W), lightDirection, color, cameraPosition);
}

void Wall::LoadModel()
{
	static bool wasLoaded = false;
	if (wasLoaded) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load(GetModelPath(ModelAttribute::SideWall), nullptr);

	_ASSERT_EXPR(result, L"Failed : Load boss's missile model.");

	pModel = Donya::StaticMesh::Create(loader);

	if (!pModel)
	{
		_ASSERT_EXPR(0, L"Failed : Load boss's missile model.");
		exit(-1);
	}

	wasLoaded = true;
}