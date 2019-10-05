#include "Player.h"

#include <algorithm>	// Use std::min().

#include "Donya/Constant.h"
#include "Donya/Loader.h"
#include "Donya/Random.h"
#include "Donya/Serializer.h"
#include "Donya/Sound.h"
#include "Donya/Template.h"
#include "Donya/Useful.h"

#if DEBUG_MODE
#include "Donya/GeometricPrimitive.h" // For drawing collision.
#endif // DEBUG_MODE

#include "Common.h"
#include "FilePath.h"
#include "Music.h"

#undef max
#undef min

#pragma region ReflectedEntity

std::shared_ptr<Donya::StaticMesh> ReflectedEntity::pModel{ nullptr };
void ReflectedEntity::LoadModel()
{
	static bool wasLoaded = false;
	if ( wasLoaded ) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::ReflectedEntity ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load player's reflection-entity model." );

	pModel = Donya::StaticMesh::Create( loader );

	_ASSERT_EXPR( pModel, L"Failed : Load player's reflection-entity model." );

	wasLoaded = true;

}

ReflectedEntity::ReflectedEntity() :
	gravity(),
	hitBox(),
	wsPos(), velocity(),
	posture( Donya::Quaternion::Identity() )
{}
ReflectedEntity::~ReflectedEntity() = default;

void ReflectedEntity::Init( float argGravity, Sphere argHitBox, Donya::Vector3 argWSPos, Donya::Vector3 argVelocity )
{
	gravity		= argGravity;
	hitBox		= argHitBox;
	wsPos		= argWSPos;
	velocity	= argVelocity;

	float radZ	= ToRadian( Donya::Random::GenerateFloat( 0.0f, 359.0f ) );
	posture		= Donya::Quaternion::Make( 0.0f, 0.0f, radZ );
}
void ReflectedEntity::Uninit()
{
	// No op.
}

void ReflectedEntity::Update()
{
	velocity.y -= gravity;

	wsPos += velocity;

	constexpr float ROT_SPEED = ToRadian( 12.0f );
	auto rotation = Donya::Quaternion::Make( Donya::Vector3::Right(), ROT_SPEED );
	posture = rotation * posture;
}

void ReflectedEntity::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const Donya::Vector4 &lightDirection, const Donya::Vector4 &cameraPos ) const
{
	if ( !pModel ) { return; }
	// else

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
	XMMATRIX T = XMMatrixTranslation( wsPos.x, wsPos.y, wsPos.z );
	XMMATRIX W = S * R * T;

	XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	pModel->Render
	(
		Float4x4( WVP ),
		Float4x4( W ),
		lightDirection,
		color,
		cameraPos
	);

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		Sphere wsBody = GetHitBox();
		wsBody.radius *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		float &scale = wsBody.radius;
		XMMATRIX colS = XMMatrixScaling( scale, scale, scale );
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * R * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 0.6f, 1.0f, 0.4f, 0.5f };

		auto InitializedSphere = []()
		{
			Donya::Geometric::Sphere sphere{};
			sphere.Init();
			return sphere;
		};
		static Donya::Geometric::Sphere sphere = InitializedSphere();
		sphere.Render
		(
			Float4x4( colWVP ),
			Float4x4( colW ),
			lightDirection,
			colColor
		);

	}

#endif // DEBUG_MODE
}

bool ReflectedEntity::ShouldErase() const
{
	return ( wsPos.y < 0.0f + hitBox.radius ) ? true : false;
}

Sphere ReflectedEntity::GetHitBox() const
{
	Sphere wsHitBox = hitBox;
	wsHitBox.pos += wsPos;
	return wsHitBox;
}

// regioni ReflectedEntity
#pragma endregion

struct PlayerParameter final : public Donya::Singleton<PlayerParameter>
{
	friend class Donya::Singleton<PlayerParameter>;
public:
	int		stunFrame;
	float	changeLaneSpeed;// Horizontal move speed.
	float	chargeSpeed;	// MAX is 1.0f.
	float	fallResistance;	// Resist to gravity(will calc to "gravity * ( 1 - resistance * charge )"). this will affected by charge.
	float	gravity;		// This is not affected by charge.
	float	jumpResistance;	// Resist to jumpStrength(will calc to "strength * ( 1 - resistance * charge )"). this will affected by charge.
	float	jumpStrength;	// Init speed of jump. This is not affected by charge.
	float	runSpeedUsual;	// Running speed when not charging.
	float	runSpeedSlow;	// Running speed when charging.
	AABB	hitBox{};		// Store local-space.
	Donya::Vector3 generateReflectionOffset;
	Player::CollideResult reflection;
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( hitBox ),
			CEREAL_NVP( changeLaneSpeed ),
			CEREAL_NVP( chargeSpeed ),
			CEREAL_NVP( fallResistance ),
			CEREAL_NVP( gravity ),
			CEREAL_NVP( jumpResistance ),
			CEREAL_NVP( jumpStrength ),
			CEREAL_NVP( runSpeedUsual ),
			CEREAL_NVP( runSpeedSlow )
		);

		if ( 1 <= version )
		{
			archive
			(
				CEREAL_NVP( stunFrame ),
				CEREAL_NVP( generateReflectionOffset ),
				CEREAL_NVP( reflection.gravity ),
				CEREAL_NVP( reflection.hitBox ),
				CEREAL_NVP( reflection.velocity )
			);
		}
		if ( 2 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Player";
public:

	void LoadParameter( bool isBinary = true )
	{
		Serializer::Extension ext = ( isBinary )
		? Serializer::Extension::BINARY
		: Serializer::Extension::JSON;
		std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

		Serializer seria;
		seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
	}

#if USE_IMGUI

	void SaveParameter()
	{
		Serializer::Extension bin = Serializer::Extension::BINARY;
		Serializer::Extension json = Serializer::Extension::JSON;
		std::string binPath = GenerateSerializePath( SERIAL_ID, bin );
		std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

		Serializer seria;
		seria.Save( bin, binPath.c_str(), SERIAL_ID, *this );
		seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
	}

	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( u8"プレイヤー" ) )
			{
				if ( ImGui::TreeNode( u8"当たり判定" ) )
				{
					auto EnumAABBParamToImGui = []( AABB *pAABB )
					{
						if ( !pAABB ) { return; }
						// else

						constexpr float RANGE = 64.0f;
						ImGui::SliderFloat3( u8"原点のオフセット（ＸＹＺ）",	&pAABB->pos.x,	-RANGE,	RANGE );
						ImGui::SliderFloat3( u8"サイズの半分（ＸＹＺ）",		&pAABB->size.x,	0.0f,	RANGE );

						bool &exist = pAABB->exist;
						std::string caption = u8"当たり判定：";
						caption += ( exist ) ? u8"あり" : u8"なし";
						ImGui::Checkbox( caption.c_str(), &exist );
					};

					EnumAABBParamToImGui( &hitBox );

					ImGui::TreePop();
				}

				if ( ImGui::TreeNode( u8"走行関連" ) )
				{
					static int changeLaneFrame =
					( ZeroEqual( changeLaneSpeed ) )
					? 1
					: scast<int>( 1.0f / changeLaneSpeed );
					ImGui::SliderInt( u8"レーン変更にかかる時間（フレーム）", &changeLaneFrame, 1, 120 );
					changeLaneSpeed = 1.0f / scast<float>( changeLaneFrame );

					ImGui::SliderFloat( u8"手前への速度・通常",		&runSpeedUsual,	0.01f, 64.0f );
					ImGui::SliderFloat( u8"手前への速度・チャージ中",	&runSpeedSlow,	0.01f, 64.0f );

					ImGui::TreePop();
				}

				if ( ImGui::TreeNode( u8"ジャンプ関連" ) )
				{
					static int chargeFrame =
					( ZeroEqual( chargeSpeed ) )
					? 1
					: scast<int>( 1.0f / chargeSpeed );
					ImGui::SliderInt( u8"チャージにかかる時間（フレーム）", &chargeFrame, 1, 120 );
					chargeSpeed = 1.0f / scast<float>( chargeFrame );

					ImGui::SliderFloat( u8"ジャンプの初速", &jumpStrength, 0.01f, 512.0f );
					ImGui::SliderFloat( u8"ジャンプ抵抗力（チャージ量の影響を受ける）", &jumpResistance, 0.01f, 512.0f );
					ImGui::Text( u8"ジャンプ初速 ＝ ジャンプ初速 * ( 1.0f - ジャンプ抵抗力 * チャージ量 )" );

					ImGui::SliderFloat( u8"重力", &gravity, 0.0f, 16.0f );
					ImGui::SliderFloat( u8"重力抵抗力（チャージ量の影響を受ける）", &fallResistance, 0.001f, 0.99f );
					ImGui::Text( u8"重力 ＝ 重力 * ( 1.0f - 重力抵抗力 * チャージ量 )" );

					ImGui::TreePop();
				}

				ImGui::SliderInt( u8"気絶時間（フレーム）", &stunFrame, 1, 120 );

				if ( ImGui::TreeNode( u8"反射するやつ関連" ) )
				{
					if ( ImGui::TreeNode( u8"当たり判定" ) )
					{
						auto EnumSphereParamToImGui = []( Sphere *pSphere )
						{
							if ( !pSphere ) { return; }
							// else

							constexpr float RANGE = 64.0f;
							ImGui::SliderFloat3( u8"原点のオフセット（ＸＹＺ）", &pSphere->pos.x, -RANGE, RANGE );
							ImGui::SliderFloat3( u8"半径", &pSphere->radius, 0.01f, RANGE );

							pSphere->exist = true;
						};

						EnumSphereParamToImGui( &reflection.hitBox );

						ImGui::TreePop();
					}

					ImGui::SliderFloat( u8"重力", &reflection.gravity, 0.01f, 64.0f );
					ImGui::Text( "" );

					ImGui::SliderFloat3( u8"生成位置のオフセット[X:%5.3f][X:%5.3f][X:%5.3f]", &generateReflectionOffset.x, -128.0f, 128.0f );
					ImGui::SliderFloat3( u8"速度[X:%5.3f][Y:%5.3f][Z:%5.3f]", &reflection.velocity.x, -64.0f, 64.0f );

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

};

CEREAL_CLASS_VERSION( PlayerParameter, 1 )

Player::Player() :
	status( State::Run ),
	currentLane(), laneCount(),
	stunTimer(),
	charge(),
	hitBox(),
	pos(), velocity(),
	posture( 0.0f, 0.0f, 0.0f, 1.0f ),
	pModel( nullptr ),
	lanePositions()
{

}
Player::~Player()
{
	pModel.reset();

	lanePositions.clear();
	lanePositions.shrink_to_fit();
}

void Player::Init( const std::vector<Donya::Vector3> &lanes )
{
	_ASSERT_EXPR( 0 < lanes.size(), L"Error : The lanes count is must over than zero." );
	laneCount = scast<int>( lanes.size() ) - 1;
	lanePositions = lanes;

	currentLane = laneCount >> 1;

	LoadModel();
	ReflectedEntity::LoadModel();

	PlayerParameter::Get().LoadParameter();

	ApplyExternalParameter();

	posture = Donya::Quaternion::Make( 0.0f, ToRadian( 180.0f ), 0.0f );

	RunInit();
}
void Player::Uninit()
{
	// No op.
}

#if DEBUG_MODE
#include "Donya/Keyboard.h"
#endif // DEBUG_MODE

void Player::Update( Input input )
{
#if USE_IMGUI

	PlayerParameter::Get().UseImGui();
	ApplyExternalParameter();
	ShowParamToImGui();

#endif // USE_IMGUI

	ChooseCurrentStateUpdate( input );

	ApplyVelocity();
}

void Player::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const Donya::Vector4 &lightDirection, const Donya::Vector4 &cameraPos ) const
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

	pModel->Render
	(
		Float4x4( WVP ),
		Float4x4( W ),
		lightDirection,
		color,
		cameraPos
	);

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		XMMATRIX colS = XMMatrixScaling( wsBody.size.x, wsBody.size.y, wsBody.size.z );
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * R * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 0.6f, 1.0f, 0.4f, 0.5f };

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

AABB Player::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += pos;
	return wsHitBox;
}

Player::CollideResult MakeFetchedResult()
{
	Player::CollideResult rv{};
	rv.wsPos = {};
	rv.shouldGenerateBullet = false;
	
	auto &param	= PlayerParameter::Get();
	rv.gravity	= param.reflection.gravity;
	rv.hitBox	= param.reflection.hitBox;
	rv.velocity	= param.reflection.velocity;

	return rv;
}
Player::CollideResult Player::ReceiveImpact( bool canReflection )
{
	CollideResult rv = MakeFetchedResult();
	rv.wsPos = pos + PlayerParameter::Get().generateReflectionOffset;
	rv.shouldGenerateBullet = false;

	if ( !canReflection || !IsJumping() )
	{
		MakeStun();
		return rv;
	}
	// else

	if ( !IsFullCharged() )
	{
		MakeStun();
		return rv;
	}
	// else

	rv.shouldGenerateBullet = true;
	return rv;
}

void Player::LoadModel()
{
	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Player ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load player model." );

	pModel = Donya::StaticMesh::Create( loader );

	_ASSERT_EXPR( pModel, L"Failed : Load player model." );
}

void Player::ApplyExternalParameter()
{
	hitBox = PlayerParameter::Get().hitBox;
}

void Player::ChooseCurrentStateUpdate( Input input )
{
	switch ( status )
	{
	case Player::State::Run:	RunUpdate( input );		break;
	case Player::State::Charge:	ChargeUpdate( input );	break;
	case Player::State::Jump:	JumpUpdate( input );	break;
	case Player::State::Stun:	StunUpdate( input );	break;
	default: break;
	}
}

void Player::RunInit()
{
	status = State::Run;

	velocity.z = -PlayerParameter::Get().runSpeedUsual;
}
void Player::RunUpdate( Input input )
{
	ChangeLaneIfRequired( input );
	HorizontalMove();

	if ( !IsCharging()&& IsCloseToCurrentLane( pos ) && input.doCharge )
	{
		ChargeInit();
	}
}

void Player::ChargeInit()
{
	status = State::Charge;

	charge = 0.0f;

	velocity.z = -PlayerParameter::Get().runSpeedSlow;

	Donya::Sound::Play( Music::PlayerCharge );
}
void Player::ChargeUpdate( Input input )
{
	charge += PlayerParameter::Get().chargeSpeed;
	charge = std::min( 1.0f, charge );

	if ( !input.doCharge )
	{
		JumpInit();
	}
}
bool Player::IsCharging() const
{
	return ( status == State::Charge ) ? true : false;
}
bool Player::IsFullCharged() const
{
	return ( charge < 1.0f ) ? false : true;
}

void Player::ChangeLaneIfRequired( Input input )
{
	int  moveSign = Donya::SignBit( input.stick.x );
	if ( moveSign == 0 ) { return; }
	// else

	if ( IsChangingLane() ) { return; }
	// else

	auto  CanMoveTo = [&]( int moveSign )->bool
	{
		if ( moveSign == 0 ) { return false; }
		// else
		if ( moveSign < 0 && currentLane <= 0 ) { return false; }
		// else
		if ( moveSign > 0 && currentLane >= laneCount ) { return false; }
		// else
		return true;
	};
	if ( !CanMoveTo( moveSign ) ) { return; }
	// else

	float distanceBetweenLane = fabsf( lanePositions[currentLane].x - lanePositions[currentLane + moveSign].x );
	velocity.x = distanceBetweenLane * PlayerParameter::Get().changeLaneSpeed * moveSign;

	currentLane += moveSign;
}
void Player::HorizontalMove()
{
	if ( IsCloseToCurrentLane( pos ) )
	{
		velocity.x = 0.0f;
	}
}
bool Player::IsCloseToCurrentLane( const Donya::Vector3 &wsJudgePos ) const
{
	float destination = lanePositions[currentLane].x;
	float applicableMargin = PlayerParameter::Get().changeLaneSpeed * 0.1f;

	return ( fabsf( destination - wsJudgePos.x ) <= applicableMargin ) ? true : false;
}
bool Player::IsChangingLane() const
{
	return IsCloseToCurrentLane( pos ) ? false : true;
}

float Player::CalcGravity()
{
	auto &param = PlayerParameter::Get();

	float resistance = param.fallResistance * charge;
	float addGravity = param.gravity;
	addGravity *= 1.0f - resistance;
	return addGravity;
}
void Player::JumpInit()
{
	status = State::Jump;

	auto &param = PlayerParameter::Get();
	velocity.y = param.jumpStrength * ( 1.0f - param.jumpResistance * charge );

	Donya::Sound::Play( Music::PlayerJump );
}
void Player::JumpUpdate( Input input )
{
	velocity.y -= CalcGravity();

	if ( pos.y + velocity.y <= 0 )
	{
		Landing();

		if ( !IsStunning() )
		{
			RunInit();
		}
	}
}
bool Player::IsJumping() const
{
	return ( status == State::Jump ) ? true : false;
}

void Player::Landing()
{
	charge		= 0.0f;
	pos.y		= 0.0f;
	velocity.y	= 0.0f;

	Donya::Sound::Play( Music::PlayerLanding );
}

void Player::ApplyVelocity()
{
	pos += velocity;
}

void Player::MakeStun()
{
	stunTimer = PlayerParameter::Get().stunFrame;

	charge		= 0.0f;
	velocity.z	= 0.0f;

	status = State::Stun;
}
void Player::StunUpdate( Input input )
{
	HorizontalMove();

	// Use falling process.
	if ( 0.0f < pos.y )
	{
		JumpUpdate( input );
	}

#if DEBUG_MODE
	Donya::Quaternion tmpRot = Donya::Quaternion::Make( -Donya::Vector3::Right(), ToRadian( 12.0f ) );
	posture = tmpRot * posture;
#endif // DEBUG_MODE

	stunTimer--;
	if ( stunTimer <= 0 )
	{
		stunTimer = 0;
		RunInit();

	#if DEBUG_MODE
		posture = Donya::Quaternion::Make( 0.0f, ToRadian( 180.0f ), 0.0f );
	#endif // DEBUG_MODE
	}
}
bool Player::IsStunning() const
{
	return ( status == State::Stun ) ? true : false;
}

#if USE_IMGUI

void Player::ShowParamToImGui() const
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"プレイヤー・パラメータ" ) )
		{
			std::string statusStr{ u8"今のステータス：" };
			switch ( status )
			{
			case Player::State::Run:	statusStr += u8"走る";		break;
			case Player::State::Charge:	statusStr += u8"チャージ";	break;
			case Player::State::Jump:	statusStr += u8"ジャンプ";	break;
			case Player::State::Stun:	statusStr += u8"スタン";		break;
			default: break;
			}
			ImGui::Text( statusStr.c_str() );
			ImGui::Text( "" );

			ImGui::Text( u8"今のレーン：%d（０始まり・左から）", currentLane );
			ImGui::Text( u8"レーンの数：%d（１始まり）", laneCount );
			ImGui::Text( "" );
			
			ImGui::Text( u8"チャージ量：%5.3f", charge );
			ImGui::Text( "" );
			
			ImGui::Text( u8"位置[X:%5.3f][Y:%5.3f][Z:%5.3f]", pos.x, pos.y, pos.z );
			ImGui::Text( u8"速度[X:%5.3f][Y:%5.3f][Z:%5.3f]", velocity.x, velocity.y, velocity.z );
			ImGui::Text( "" );

			ImGui::TreePop();
		}

		ImGui::End();
	}
}

#endif // USE_IMGUI
