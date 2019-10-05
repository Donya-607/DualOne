#include "Boss.h"

#include <algorithm>	// Use std::remove_if.

#include "Donya/Constant.h"
#include "Donya/Loader.h"
#include "Donya/Random.h"
#include "Donya/Useful.h"

#if DEBUG_MODE
#include "Donya/GeometricPrimitive.h" // For drawing collision.
#include "Donya/Keyboard.h" // For generate missile trigger.
#endif // DEBUG_MODE

#include "Common.h"
#include "FilePath.h"

#pragma region Missile

Missile Missile::parameter{};
std::shared_ptr<Donya::StaticMesh> Missile::pModel{ nullptr };

void Missile::LoadModel()
{
	static bool wasLoaded = false;
	if ( wasLoaded ) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Missile ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load boss's missile model." );

	pModel = Donya::StaticMesh::Create( loader );

	if ( !pModel )
	{
		_ASSERT_EXPR( 0, L"Failed : Load boss's missile model." );
		exit( -1 );
	}

	wasLoaded = true;
}

void Missile::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, parameter );
}

#if USE_IMGUI

void Missile::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin  );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, parameter );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, parameter );
}

void Missile::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ミサイル" ) )
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

				EnumAABBParamToImGui( &parameter.hitBox );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"性能" ) )
			{
				ImGui::SliderInt( u8"消えるまでの時間", &parameter.aliveFrame, 1, 360 );

				ImGui::SliderFloat3( u8"移動速度", &parameter.velocity.x, 0.1f, 32.0f );
				if ( 0.0f < parameter.velocity.z ) { parameter.velocity.z *= -1.0f; }

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

Missile::Missile() :
	aliveFrame( 0 ), waitFrame( 0 ),
	hitBox(),
	pos(), velocity(),
	posture(),
	wasHitToOther( false )
{

}
Missile::~Missile() = default;

void Missile::Init( const Donya::Vector3 &wsAppearPos )
{
	// Apply the external paramter.
	*this		= parameter;

	pos			= wsAppearPos;

	posture		= Donya::Quaternion::Make( Donya::Vector3::Up(), ToRadian( 180.0f ) );
}

void Missile::Uninit()
{
	// No op.
}

void Missile::Update()
{
	aliveFrame--;

	Move();

#if DEBUG_MODE

	auto rotQ = Donya::Quaternion::Make( Donya::Vector3::Front(), ToRadian( 12.0f ) );
	posture = rotQ * posture;

#endif // DEBUG_MODE
}

void Missile::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
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

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		XMMATRIX colS = XMMatrixScaling( wsBody.size.x, wsBody.size.y, wsBody.size.z );
		// XMMATRIX colR = XMMatrixIdentity();
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 1.0f, 0.3f, 0.2f, 0.5f };

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

AABB Missile::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += GetPos();
	return wsHitBox;
}

bool Missile::ShouldErase() const
{
	return ( aliveFrame <= 0 || wasHitToOther ) ? true : false;
}

void Missile::HitToOther() const
{
	wasHitToOther = true;
}

void Missile::Move()
{
	pos += velocity;
}

// region Missile
#pragma endregion

#pragma region AttackParam

AttackParam::AttackParam() :
	counterMax( 0 ), untilAttackFrame( 0 ), reuseFrame( 0 ),
	intervals(), obstaclePatterns()
{}
AttackParam::~AttackParam()
{
	obstaclePatterns.clear();
	obstaclePatterns.shrink_to_fit();
}

void AttackParam::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void AttackParam::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void AttackParam::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ボスの攻撃パターン" ) )
		{
			ImGui::SliderInt( u8"攻撃条件に使うカウンタの最大値", &counterMax, 100, 2048 );
			ImGui::SliderInt( u8"攻撃開始までの待機時間（フレーム）", &untilAttackFrame, 1, 2048 );
			ImGui::SliderInt( u8"攻撃後の待機時間（フレーム）", &reuseFrame, 1, 2048 );

			static int targetNo{};
			std::string attackNameU8{};
			switch ( targetNo )
			{
			case AttackKind::Missile:  attackNameU8 = u8"ミサイル"; break;
			case AttackKind::Obstacle: attackNameU8 = u8"障害物"; break;
			default: attackNameU8 = u8"Error !"; break;
			}

			std::string caption{ u8"攻撃の種類：" };
			ImGui::SliderInt( ( caption + attackNameU8 ).c_str(), &targetNo, 0, AttackKind::ATTACK_KIND_COUNT - 1 );

			ImGui::SliderInt( u8"攻撃の間隔（フレーム）", &intervals[targetNo], 1, 999 );

			if ( ImGui::TreeNode( u8"障害物のパターン" ) )
			{
				ImGui::Text( u8"０始まり，左から数える" );
				ImGui::Text( "" );

				if ( ImGui::Button( u8"パターンを増やす" ) )
				{
					obstaclePatterns.push_back( {} );
				}
				if ( !obstaclePatterns.empty() )
				{
					if ( ImGui::Button( u8"パターンを減らす" ) )
					{
						obstaclePatterns.pop_back();
					}
				}
				ImGui::Text( "" );

				const size_t PATTERN_COUNT = obstaclePatterns.size();
				for ( size_t i = 0; i < PATTERN_COUNT; ++i  )
				{
					std::string patternName = "パターン[" + std::to_string( i ) + "]";
					if ( ImGui::TreeNode( Donya::MultiToUTF8( patternName ).c_str() ) )
					{
						auto &patterns = obstaclePatterns[i];

						if ( ImGui::Button( u8"レーンを増やす" ) )
						{
							patterns.push_back( {} );
						}
						if ( !patterns.empty() )
						{
							if ( ImGui::Button( u8"レーンを減らす" ) )
							{
								patterns.pop_back();
							}
						}

						std::string caption{};
						const size_t COUNT = patterns.size();
						for ( size_t j = 0; j < COUNT; ++j )
						{
							caption = "[" + std::to_string( j ) + "]";
							ImGui::SliderInt( caption.c_str(), &( patterns[j] ), 0, 1 );
						}

						ImGui::TreePop();
					}
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

// region AttackParam
#pragma endregion


#pragma region Boss

Boss::Boss() :
	attackTimer(), waitReuseFrame(),
	maxDistanceToTarget(),
	hitBox(),
	pos(), velocity(), missileOffset(),
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

	Missile::LoadParameter();
	Missile::LoadModel();

	AttackParam::Get().LoadParameter();

	waitReuseFrame = AttackParam::Get().untilAttackFrame;

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

void Boss::Update( const Donya::Vector3 &wsAttackTargetPos )
{
#if USE_IMGUI

	UseImGui();
	Missile::UseImGui();
	AttackParam::Get().UseImGui();

#endif // USE_IMGUI

	// I think the attack is should use after the move.

	Move( wsAttackTargetPos );

	LotteryAttack( wsAttackTargetPos );

	UpdateMissiles();
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
		// XMMATRIX colR = XMMatrixIdentity();
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * colT;

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

const std::vector<Missile> &Boss::FetchReflectableMissiles() const
{
	return missiles;
}

void Boss::LoadModel()
{
	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Boss ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load boss model." );

	pModel = Donya::StaticMesh::Create( loader );

	_ASSERT_EXPR( pModel, L"Failed : Load boss model." );
}

void Boss::Move( const Donya::Vector3 &wsAttackTargetPos )
{
	pos += velocity;

	float distance = wsAttackTargetPos.z - pos.z;
	if ( maxDistanceToTarget < fabsf( distance ) )
	{
		// Place to back than target-position.
		pos.z = wsAttackTargetPos.z + maxDistanceToTarget;
	}
}

void Boss::LotteryAttack( const Donya::Vector3 &wsAttackTargetPos )
{
	if ( 0 < waitReuseFrame )
	{
		waitReuseFrame--;
		return;
	}
	// else

	attackTimer++; // This timer's count does not will be zero.

	constexpr int COUNT = AttackParam::AttackKind::ATTACK_KIND_COUNT;
	const auto &PARAM = AttackParam::Get();

	std::vector<int> chosenIndices{};
	for ( int i = 0; i < COUNT; ++i )
	{
		if ( attackTimer % PARAM.intervals[i] == 0 )
		{
			chosenIndices.emplace_back( i );
		}
	}

	constexpr int NOT_USE_NO = -1;
	int useAttackNo = NOT_USE_NO;

	if ( !chosenIndices.empty() )
	{
		if ( 1 == chosenIndices.size() )
		{
			useAttackNo = chosenIndices.front();
		}
		else
		{
			int random = Donya::Random::GenerateInt( chosenIndices.size() );
			useAttackNo = chosenIndices[random];
		}
	}

	if ( useAttackNo != NOT_USE_NO )
	{
		switch ( useAttackNo )
		{
		case AttackParam::AttackKind::Missile: ShootMissile( wsAttackTargetPos ); break;
		default: break;
		}

		waitReuseFrame = PARAM.reuseFrame;
	}

	if ( PARAM.counterMax <= attackTimer )
	{
		attackTimer = 0;
	}
}

Donya::Vector3 Boss::LotteryLanePosition()
{
	const size_t laneCount = lanePositions.size();
	_ASSERT_EXPR( 0 < laneCount, L"The lane count is must over than zero !" );

	int index = Donya::Random::GenerateInt( 0, laneCount );
	return lanePositions[index];
}

void Boss::ShootMissile( const Donya::Vector3 &wsAttackTargetPos )
{
	Donya::Vector3 appearPos = LotteryLanePosition();
	appearPos.z = pos.z;

	Donya::Vector3 dir = appearPos - pos;
	appearPos.x += missileOffset.x * Donya::SignBit( dir.x );
	appearPos.y += missileOffset.y;
	appearPos.z += missileOffset.z;

	missiles.push_back( {} );
	missiles.back().Init( appearPos );
}

void Boss::UpdateMissiles()
{
	for ( auto &it : missiles )
	{
		it.Update();
	}

	auto eraseItr = std::remove_if
	(
		missiles.begin(), missiles.end(),
		[]( Missile &element )
		{
			if ( element.ShouldErase() )
			{
				element.Uninit();
				return true;
			}
			// else
			return false;
		}
	);
	missiles.erase( eraseItr, missiles.end() );
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
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
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
				ImGui::SliderFloat3( u8"移動速度", &velocity.x, 0.1f, 32.0f );
				if ( 0.0f < velocity.z ) { velocity.z *= -1.0f; }

				ImGui::SliderFloat( u8"プレイヤーとの距離の最大", &maxDistanceToTarget, 1.0f, 1024.0f );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"攻撃関連" ) )
			{
				ImGui::SliderFloat3( u8"ミサイル発射位置のオフセット", &missileOffset.x, -128.0f, 128.0f );

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
