#include "Boss.h"

#include "Donya/Constant.h"
#include "Donya/Loader.h"
#include "Donya/Useful.h"

#if DEBUG_MODE
#include "Donya/GeometricPrimitive.h" // For drawing collision.
#endif // DEBUG_MODE

#include "Common.h"
#include "FilePath.h"

#pragma region Missile

void Missile::Init()
{
	pos = Donya::Vector3( 0.0f, 0.0f, 0.0f );
	velocity = Donya::Vector3( 0.0f, 0.0f, 0.0f );
	scale = Donya::Vector3( 1.0f, 1.0f, 1.0f );
	state = INITIALIZE;
}

void Missile::Uninit()
{

}

void Missile::Update( Donya::Vector3 bossPos )
{
	Move( bossPos );
}

void Missile::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
{
	// TODO:Drawing collision box(or sphere).
}

void Missile::Move( Donya::Vector3 bossPos )
{
	switch ( state )
	{
	case Missile::NOT_ENABLE:

		break;
	case Missile::INITIALIZE:

//		break;	意図的なコメントアウト
	case Missile::PREP_MOVE:

		break;
	case Missile::PREP_STOP:

		break;
	case Missile::ATTACK_MOVE:

		break;
	case Missile::END:

		break;
	default:
		break;
	}
	pos += velocity;
}

// region Missile
#pragma endregion

#pragma region Boss

Boss::Boss() :
	hitBox(),
	pos(), velocity(),
	posture(),
	pModel( nullptr ),
	missiles(), lanePositions()
{}
Boss::~Boss()
{
	pModel.reset();

	missiles.clear();
	missiles.shrink_to_fit();
	lanePositions.clear();
	lanePositions.shrink_to_fit();
}

void Boss::Init( float initDistanceFromOrigin, const std::vector<Donya::Vector3> &registerLanePositions )
{
	LoadParameter();
	LoadModel();

	pos = Donya::Vector3{ 0.0f, 0.0f, initDistanceFromOrigin };
	
	lanePositions = registerLanePositions;

	posture = Donya::Quaternion::Make( 0.0f, ToRadian( 180.0f ), 0.0f );
}

void Boss::Uninit()
{
	for ( auto &it : missiles )
	{
		it.Uninit();
	}
}

void Boss::Update()
{
#if USE_IMGUI

	UseImGui();

#endif // USE_IMGUI

	Move();

	for ( auto &it : missiles )
	{
		it.Update( pos );
	}
}

void Boss::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
{
	using namespace DirectX;

	auto Matrix		= []( const XMFLOAT4X4 &matrix )
	{
		return XMLoadFloat4x4( &matrix );
	};
	auto Float4x4	= []( const XMMATRIX &M )
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4( &matrix, M );
		return matrix;
	};

	XMMATRIX S = XMMatrixIdentity();
	XMMATRIX R = Matrix( posture.RequireRotationMatrix() );
	XMMATRIX T = XMMatrixTranslation( pos.x, pos.y, pos.z );
	XMMATRIX W = S * R * T;
	XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	pModel->Render( Float4x4( WVP ), Float4x4( W ), lightDirection, color, cameraPosition, isEnableFill );

	for ( auto &it : missiles )
	{
		it.Draw( matView, matProjection, lightDirection, cameraPosition );
	}

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		XMMATRIX colS = XMMatrixScaling( wsBody.size.x, wsBody.size.y, wsBody.size.z );
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * R * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 1.0f, 0.6f, 0.2f, 0.5f };

		auto InitializedCube = []()
		{
			Donya::Geometric::Cube cube{};
			cube.Init();
			return cube;
		};
		static Donya::Geometric::Cube cube = InitializedCube();
		cube.Render
		(
			Float4x4( colWVP ),
			Float4x4( colW ),
			lightDirection,
			colColor
		);

	}

#endif // DEBUG_MODE
}

AABB Boss::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += GetPos();
	return wsHitBox;
}

void Boss::LoadModel()
{
	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Boss ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load boss model." );

	pModel = Donya::StaticMesh::Create( loader );

	_ASSERT_EXPR( pModel, L"Failed : Load boss model." );
}

void Boss::Move()
{
	pos += velocity;
}

void Boss::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void Boss::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin, binPath.c_str(), SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void Boss::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ボス" ) )
		{
			if ( ImGui::TreeNode( u8"当たり判定" ) )
			{
				auto EnumAABBParamToImGui = []( AABB *pAABB )
				{
					if ( !pAABB ) { return; }
					// else

					constexpr float RANGE = 64.0f;
					ImGui::SliderFloat3( u8"原点のオフセット（ＸＹＺ）", &pAABB->pos.x, -RANGE, RANGE );
					ImGui::SliderFloat3( u8"サイズの半分（ＸＹＺ）", &pAABB->size.x, 0.0f, RANGE );

					bool &exist = pAABB->exist;
					std::string caption = u8"当たり判定：";
					caption += ( exist ) ? u8"あり" : u8"なし";
					ImGui::Checkbox( caption.c_str(), &exist );
				};

				EnumAABBParamToImGui( &hitBox );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"身体性能" ) )
			{
				ImGui::SliderFloat( u8"移動速度", &velocity.z, 0.1f, 32.0f );
				velocity.z *= -1.0f;

				if ( ImGui::TreeNode( "Detail" ) )
				{
					ImGui::SliderFloat3( u8"移動速度", &velocity.x, 0.1f, 32.0f );
					velocity.z *= -1.0f;

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"ファイル" ) )
			{
				static bool isBinary = true;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "読み込み " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"保存" ) )
				{
					SaveParameter();
				}
				if ( ImGui::Button( Donya::MultiToUTF8( loadStr ).c_str() ) )
				{
					LoadParameter( isBinary );
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}
}

#endif // USE_IMGUI

// region Boss
#pragma endregion
