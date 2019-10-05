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
		if ( ImGui::TreeNode( u8"�~�T�C��" ) )
		{
			if ( ImGui::TreeNode( u8"�����蔻��" ) )
			{
				auto EnumAABBParamToImGui = []( AABB *pAABB )
				{
					if ( !pAABB ) { return; }
					// else

					constexpr float RANGE = 64.0f;
					ImGui::SliderFloat3( u8"���_�̃I�t�Z�b�g�i�w�x�y�j", &pAABB->pos.x, -RANGE, RANGE );
					ImGui::SliderFloat3( u8"�T�C�Y�̔����i�w�x�y�j", &pAABB->size.x, 0.0f, RANGE );

					bool &exist = pAABB->exist;
					std::string caption = u8"�����蔻��F";
					caption += ( exist ) ? u8"����" : u8"�Ȃ�";
					ImGui::Checkbox( caption.c_str(), &exist );
				};

				EnumAABBParamToImGui( &parameter.hitBox );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"���\" ) )
			{
				ImGui::SliderInt( u8"������܂ł̎���", &parameter.aliveFrame, 1, 360 );

				ImGui::SliderFloat3( u8"�ړ����x", &parameter.velocity.x, 0.1f, 32.0f );
				if ( 0.0f < parameter.velocity.z ) { parameter.velocity.z *= -1.0f; }

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"�t�@�C��" ) )
			{
				static bool isBinary = true;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "�ǂݍ��� " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"�ۑ�" ) )
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
	status( State::NOT_ENABLE ),
	aliveFrame( 0 ), waitFrame( 0 ),
	hitBox(),
	pos(), velocity(),
	posture(),
	isShotFromRight( false )
{

}
Missile::~Missile() = default;

void Missile::Init( const Donya::Vector3 &wsAppearPos )
{
	// Apply the external paramter.
	*this		= parameter;

	status		= State::INITIALIZE;
	pos			= wsAppearPos;

	posture		= Donya::Quaternion::Make( Donya::Vector3::Up(), ToRadian( 180.0f ) );
}

void Missile::Uninit()
{

}

void Missile::Update( Donya::Vector3 bossPos )
{
	aliveFrame--;

	Move( bossPos );

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
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * R * colT;

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
	return ( aliveFrame <= 0 ) ? true : false;
}

void Missile::Move( Donya::Vector3 bossPos )
{
	switch ( status )
	{
	case State::NOT_ENABLE:

		break;
	case State::INITIALIZE:

//		break;	�Ӑ}�I�ȃR�����g�A�E�g
	case State::PREP_MOVE:

		break;
	case State::PREP_STOP:

		break;
	case State::ATTACK_MOVE:

		break;
	case State::END:

		break;
	default:
		break;
	}
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
		if ( ImGui::TreeNode( u8"�{�X�̍U���p�^�[��" ) )
		{
			ImGui::SliderInt( u8"�U�������Ɏg���J�E���^�̍ő�l", &counterMax, 100, 2048 );
			ImGui::SliderInt( u8"�U���J�n�܂ł̑ҋ@���ԁi�t���[���j", &untilAttackFrame, 1, 2048 );
			ImGui::SliderInt( u8"�U����̑ҋ@���ԁi�t���[���j", &reuseFrame, 1, 2048 );

			static int targetNo{};
			std::string attackNameU8{};
			switch ( targetNo )
			{
			case AttackKind::Missile:  attackNameU8 = u8"�~�T�C��"; break;
			case AttackKind::Obstacle: attackNameU8 = u8"��Q��"; break;
			default: attackNameU8 = u8"Error !"; break;
			}

			std::string caption{ u8"�U���̎�ށF" };
			ImGui::SliderInt( ( caption + attackNameU8 ).c_str(), &targetNo, 0, AttackKind::ATTACK_KIND_COUNT - 1 );

			ImGui::SliderInt( u8"�U���̊Ԋu�i�t���[���j", &intervals[targetNo], 1, 999 );

			if ( ImGui::TreeNode( u8"��Q���̃p�^�[��" ) )
			{
				ImGui::Text( u8"�O�n�܂�C�����琔����" );
				ImGui::Text( "" );

				if ( ImGui::Button( u8"�p�^�[���𑝂₷" ) )
				{
					obstaclePatterns.push_back( {} );
				}
				if ( !obstaclePatterns.empty() )
				{
					if ( ImGui::Button( u8"�p�^�[�������炷" ) )
					{
						obstaclePatterns.pop_back();
					}
				}
				ImGui::Text( "" );

				const size_t PATTERN_COUNT = obstaclePatterns.size();
				for ( size_t i = 0; i < PATTERN_COUNT; ++i  )
				{
					std::string patternName = "�p�^�[��[" + std::to_string( i ) + "]";
					if ( ImGui::TreeNode( Donya::MultiToUTF8( patternName ).c_str() ) )
					{
						auto &patterns = obstaclePatterns[i];

						if ( ImGui::Button( u8"���[���𑝂₷" ) )
						{
							patterns.push_back( {} );
						}
						if ( !patterns.empty() )
						{
							if ( ImGui::Button( u8"���[�������炷" ) )
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

			if ( ImGui::TreeNode( u8"�t�@�C��" ) )
			{
				static bool isBinary = true;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "�ǂݍ��� " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"�ۑ�" ) )
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

void Boss::Update()
{
#if USE_IMGUI

	UseImGui();
	Missile::UseImGui();
	AttackParam::Get().UseImGui();

#endif // USE_IMGUI

	// I think the attack is should use after the move.

	Move();

	LotteryAttack();

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

void Boss::ShootMissile()
{
#if DEBUG_MODE

	_ASSERT_EXPR( 0 < lanePositions.size(), L"The lane count is must over than zero !" );
	const size_t laneCount = lanePositions.size();
	Donya::Vector3 appearPos = lanePositions[Donya::Random::GenerateInt( 0, laneCount )];
	appearPos.z = pos.z;

	Donya::Vector3 dir = appearPos - pos;
	appearPos.x += missileOffset.x * Donya::SignBit( dir.x );
	appearPos.y += missileOffset.y;
	appearPos.z += missileOffset.z;

	missiles.push_back( {} );
	missiles.back().Init( appearPos );

#endif // DEBUG_MODE


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

void Boss::LotteryAttack()
{
	if ( 0 < waitReuseFrame )
	{
		waitReuseFrame--;
		return;
	}
	// else

	attackTimer++;

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
		case AttackParam::AttackKind::Missile: ShootMissile(); break;
		default: break;
		}

		waitReuseFrame = PARAM.reuseFrame;
	}

	if ( PARAM.counterMax <= attackTimer )
	{
		attackTimer = 0;
	}
}

void Boss::UpdateMissiles()
{
	for ( auto &it : missiles )
	{
		it.Update( pos );
	}

	auto eraseItr = std::remove_if
	(
		missiles.begin(), missiles.end(),
		[]( Missile &element ) { return element.ShouldErase(); }
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
		if ( ImGui::TreeNode( u8"�{�X" ) )
		{
			if ( ImGui::TreeNode( u8"�����蔻��" ) )
			{
				auto EnumAABBParamToImGui = []( AABB *pAABB )
				{
					if ( !pAABB ) { return; }
					// else

					constexpr float RANGE = 64.0f;
					ImGui::SliderFloat3( u8"���_�̃I�t�Z�b�g�i�w�x�y�j", &pAABB->pos.x, -RANGE, RANGE );
					ImGui::SliderFloat3( u8"�T�C�Y�̔����i�w�x�y�j", &pAABB->size.x, 0.0f, RANGE );

					bool &exist = pAABB->exist;
					std::string caption = u8"�����蔻��F";
					caption += ( exist ) ? u8"����" : u8"�Ȃ�";
					ImGui::Checkbox( caption.c_str(), &exist );
				};

				EnumAABBParamToImGui( &hitBox );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"�g�̐��\" ) )
			{
				ImGui::SliderFloat3( u8"�ړ����x", &velocity.x, 0.1f, 32.0f );
				if ( 0.0f < velocity.z ) { velocity.z *= -1.0f; }

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"�U���֘A" ) )
			{
				ImGui::SliderFloat3( u8"�~�T�C�����ˈʒu�̃I�t�Z�b�g", &missileOffset.x, -128.0f, 128.0f );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"�t�@�C��" ) )
			{
				static bool isBinary = true;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "�ǂݍ��� " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"�ۑ�" ) )
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
