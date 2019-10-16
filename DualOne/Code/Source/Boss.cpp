#include "Boss.h"

#include <algorithm>	// Use std::remove_if, std::max(), min().

#include "Donya/Constant.h"
#include "Donya/Easing.h"
#include "Donya/Loader.h"
#include "Donya/Random.h"
#include "Donya/ScreenShake.h"
#include "Donya/Sound.h"
#include "Donya/Sprite.h"
#include "Donya/Useful.h"

#if DEBUG_MODE
#include "Donya/GeometricPrimitive.h" // For drawing collision.
#include "Donya/Keyboard.h" // For debug trigger.
#endif // DEBUG_MODE

#include "Common.h"
#include "FilePath.h"
#include "Effect.h"
#include "Music.h"

#undef max
#undef min

static int GetEasingKindCount()
{
	return scast<int>( Donya::Easing::Kind::ENUM_TERMINATION );
}
static int GetEasingTypeCount()
{
	return 3; // scast<int>( Donya::Easing::Type::InOut ) + 1;
}
static std::string EasingKindToStr( int easingKind )
{
	using namespace Donya::Easing;
	Kind kind = scast<Kind>( easingKind );
	switch ( kind )
	{
	case Kind::Linear:			return "Linear";
	case Kind::Back:			return "Back";
	case Kind::Bounce:			return "Bounce";
	case Kind::Circular:		return "Circular";
	case Kind::Cubic:			return "Cubic";
	case Kind::Elastic:			return "Elastic";
	case Kind::Exponential:		return "Exponential";
	case Kind::Quadratic:		return "Quadratic";
	case Kind::Quartic:			return "Quartic";
	case Kind::Quintic:			return "Quintic";
	case Kind::Smooth:			return "Smooth";
	case Kind::Sinusoidal:		return "Sinusoidal";
	case Kind::SoftBack:		return "SoftBack";
	case Kind::Step:			return "Step";
	}

	return "Error Kind !";
}
static std::string EasingTypeToStr( int easingType )
{
	using namespace Donya::Easing;
	Type type = scast<Type>( easingType );
	switch ( type )
	{
	case Type::In:		return "In";
	case Type::Out:		return "Out";
	case Type::InOut:	return "InOut";
	}

	return "Error Type !";
}

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
				ImGui::SliderFloat( u8"顔見せ・生成から待機までの移動距離", &parameter.exposingLength, 0.0f, 256.0f );
				ImGui::SliderInt( u8"顔見せ・待機時間（フレーム）", &parameter.waitFrame, 1, 360 );
				ImGui::SliderInt( u8"消えるまでの時間（フレーム）", &parameter.aliveFrame, 1, 360 );

				ImGui::SliderFloat3( u8"移動速度", &parameter.velocity.x, 0.1f, 32.0f );
				
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
	status( State::Expose ),
	aliveFrame( 0 ), waitFrame( 0 ),
	exposingLength(),
	hitBox(),
	pos(), basePos(), velocity(),
	posture(),
	wasHitToOther( false )
{}
Missile::~Missile() = default;

void Missile::Init( const Donya::Vector3 &wsAppearPos, bool useFast )
{
	// Apply the external paramter.
	*this		= parameter;

	pos			= wsAppearPos;
	basePos		= wsAppearPos;

	posture		= Donya::Quaternion::Make( Donya::Vector3::Up(), ToRadian( 180.0f ) );

	if ( useFast )
	{
		InitToFastMode();
	}
}

void Missile::Uninit()
{
	Donya::Sound::Stop( Music::BossShootMissile );
}

void Missile::Update( float basePosZ )
{
	basePos.z = basePosZ;

	switch ( status )
	{
	case Missile::State::Expose:	ExposeUpdate();	break;
	case Missile::State::Wait:		WaitUpdate();	break;
	case Missile::State::Fly:		FlyUpdate();	break;
	default: break;
	}
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
	if ( status != State::Fly ) { return AABB::Nil(); }
	// else

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

void Missile::InitToFastMode()
{
	// Setting to immediately transition to flying state.
	// And setting to immediately showing to screen.

	status		= State::Wait;
	waitFrame	= 0;

	exposingLength *= 3.5f;
}

void Missile::ExposeUpdate()
{
	Move();

	float distance = fabsf( pos.z - basePos.z );
	if ( exposingLength <= distance )
	{
		pos.z = basePos.z - exposingLength;

		status = State::Wait;
	}
}
void Missile::WaitUpdate()
{
	pos.z = basePos.z - exposingLength;

	waitFrame--;
	if ( waitFrame <= 0 )
	{
		waitFrame = 0;
		status = State::Fly;

		Donya::Sound::Play( Music::BossShootMissile );
	}
}
void Missile::FlyUpdate()
{
	aliveFrame--;

	Move();

	ParticleManager::Get().CreateSmokeOfMissileParticle( pos );

#if DEBUG_MODE

	auto rotQ = Donya::Quaternion::Make( Donya::Vector3::Front(), ToRadian( 12.0f ) );
	posture = rotQ * posture;

#endif // DEBUG_MODE
}

void Missile::Move()
{
	pos.x += velocity.x;
	pos.y += velocity.y;
	pos.z -= velocity.z;
}

// region Missile
#pragma endregion

#pragma region Obstacle

#pragma region Warning

size_t Obstacle::Warning::sprWarning{ NULL };
Obstacle::Warning Obstacle::Warning::parameter{};

void Obstacle::Warning::RegisterLaneCount( size_t newLaneCount )
{
	const size_t CURRENT_COUNT = parameter.ssPositions.size();
	if ( CURRENT_COUNT == newLaneCount ) { return; }
	// else

	parameter.ssPositions.resize( newLaneCount );
}

void Obstacle::Warning::LoadSprite()
{
	if ( sprWarning != NULL ) { return; }
	// else

	sprWarning = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::Warning ), 16U );
}

void Obstacle::Warning::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, parameter );
}

#if USE_IMGUI

void Obstacle::Warning::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin  );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, parameter );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, parameter );
}

void Obstacle::Warning::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"障害物の警告" ) )
		{
			ImGui::SliderInt( u8"表示する時間（フレーム）", &parameter.showFrame, 1, 360 );

			if ( ImGui::TreeNode( u8"描画位置" ) )
			{
				std::string caption{};
				const size_t COUNT = parameter.ssPositions.size();
				for ( size_t i = 0; i < COUNT; ++i )
				{
					caption = std::to_string( i ) + "スクリーン座標(X,Y)";
					caption = Donya::MultiToUTF8( caption );
					ImGui::SliderFloat2( caption.c_str(), &parameter.ssPositions[i].x, 0.0f, 1920.0f );
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

Obstacle::Warning::Warning() :
	showFrame(), laneNo(),
	ssPositions()
{}
Obstacle::Warning::~Warning()
{
	ssPositions.clear();
	ssPositions.shrink_to_fit();
}

void Obstacle::Warning::Init( int laneNumber )
{
	// Apply the external paramter.
	*this = parameter;

	laneNo = laneNumber;
}

void Obstacle::Warning::Update()
{
	showFrame--;
}

void Obstacle::Warning::Draw() const
{
	if ( showFrame <= 0 ) { return; }
	if ( scast<int>( ssPositions.size() ) <= laneNo ) { return; } // safety.
	// else

	float oldDepth = Donya::Sprite::GetDrawDepth();
	Donya::Sprite::SetDrawDepth( 0.0f );

	Donya::Sprite::Draw
	(
		sprWarning,
		ssPositions[laneNo].x,
		ssPositions[laneNo].y,
		0.0f, 0.7f
	);

	Donya::Sprite::SetDrawDepth( oldDepth );
}

// region Warning
#pragma endregion

Obstacle Obstacle::parameter{};
std::shared_ptr<Donya::StaticMesh> Obstacle::pModel{ nullptr };

void Obstacle::LoadModel()
{
	static bool wasLoaded = false;
	if ( wasLoaded ) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Obstacle ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load obstacle model." );

	pModel = Donya::StaticMesh::Create( loader );

	if ( !pModel )
	{
		_ASSERT_EXPR( 0, L"Failed : Load obstacle model." );
		exit( -1 );
	}

	wasLoaded = true;
}

void Obstacle::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, parameter );
}

#if USE_IMGUI

void Obstacle::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin  );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, parameter );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, parameter );
}

void Obstacle::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"障害物" ) )
		{
			ImGui::SliderFloat( u8"Ｚ軸の速度（位置に足され，減速のように機能します）", &parameter.decelSpeed, -8.0f, 8.0f );
			ImGui::Text( "" );

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

Obstacle::Obstacle() :
	decelSpeed(),
	hitBox(),
	warning(),
	pos(),
	posture(),
	wasHitToOther( false )
{

}
Obstacle::~Obstacle() = default;

void Obstacle::Init( int laneNumber, const Donya::Vector3 &wsAppearPos )
{
	// Apply the external paramter.
	*this		= parameter;

	pos			= wsAppearPos;
	warning.Init( laneNumber );
}
void Obstacle::Uninit()
{
	// No op.
}

void Obstacle::Update()
{
	pos.z += decelSpeed;

	warning.Update();
}

void Obstacle::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
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

	warning.Draw();

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

		constexpr XMFLOAT4 colColor{ 1.0f, 0.1f, 0.0f, 0.5f };

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

AABB Obstacle::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += GetPos();
	return wsHitBox;
}

bool Obstacle::ShouldErase() const
{
	return ( wasHitToOther ) ? true : false;
}

void Obstacle::HitToOther() const
{
	wasHitToOther = true;
}

// region Obstacle
#pragma endregion

#pragma region Beam

Beam Beam::parameter{};
std::shared_ptr<Donya::StaticMesh> Beam::pModel{ nullptr };

void Beam::LoadModel()
{
	static bool wasLoaded = false;
	if ( wasLoaded ) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Beam ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load obstacle model." );

	pModel = Donya::StaticMesh::Create( loader );

	if ( !pModel )
	{
		_ASSERT_EXPR( 0, L"Failed : Load obstacle model." );
		exit( -1 );
	}

	wasLoaded = true;
}

void Beam::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, parameter );
}

#if USE_IMGUI

void Beam::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, parameter );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, parameter );
}

void Beam::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ビーム" ) )
		{
			static int angleSwingFrame =
			( ZeroEqual( parameter.angleIncreaseSpeed ) )
			? 1
			: scast<int>( 1.0f / parameter.angleIncreaseSpeed );
			ImGui::SliderInt( u8"ビームの薙ぎ払いにかかる時間（フレーム）", &angleSwingFrame, 3, 180 );
			parameter.angleIncreaseSpeed = 1.0f / scast<float>( angleSwingFrame );
			ImGui::SliderInt( u8"薙ぎ払い終了後の待ち時間（フレーム）", &parameter.afterWaitFrame, 1, 180 );

			ImGui::SliderFloat( u8"ビームの初期角度", &parameter.beamAngleBegin, 0.0f, 360.0f );
			ImGui::SliderFloat( u8"ビームの終了角度", &parameter.beamAngleEnd, 0.0f, 360.0f );
			ImGui::SliderFloat( u8"ビームの長さ", &parameter.beamLength, 1.0f, 512.0f );
			ImGui::Text( "" );

			ImGui::SliderInt( u8"イージングの種類",   &parameter.easingKind, 0, GetEasingKindCount() - 1 );
			ImGui::SliderInt( u8"イージングのタイプ", &parameter.easingType, 0, GetEasingTypeCount() - 1 );
			std::string easingCaption = "イージング名：" + EasingKindToStr( parameter.easingKind ) + " " + EasingTypeToStr( parameter.easingType );
			easingCaption = Donya::MultiToUTF8( easingCaption );
			ImGui::Text( easingCaption.c_str() );

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

Beam::Beam() :
	status( State::Swing ),
	easingKind(), easingType(), afterWaitFrame(),
	angleIncreaseSpeed(), easeParam(),
	beamAngle(), beamAngleBegin(), beamAngleEnd(),
	beamLength(),
	hitBox(),
	basePos(), beamDestPos(),
	posture()
{}
Beam::~Beam() = default;

void Beam::Init( const Donya::Vector3 &wsAppearPos )
{
	// Apply the external paramter.
	*this = parameter;

	basePos = wsAppearPos;

	beamAngleBegin		= ToRadian( beamAngleBegin		);	// These angles stored by degree.
	beamAngleEnd		= ToRadian( beamAngleEnd		);	// These angles stored by degree.
	beamAngle			= beamAngleBegin;

	SetPosture();

	Donya::Sound::Play( Music::BossBeamShoot );
}
void Beam::Uninit()
{
	Donya::Sound::Stop( Music::BossBeamShoot );
}

void Beam::Update( float wsBasePositionZ )
{
	basePos.z = wsBasePositionZ;

	switch ( status )
	{
	case Beam::State::Swing:	AngleUpdate();	break;
	case Beam::State::Wait:		WaitUpdate();	break;
	default: break;
	}

	CalcBeamDestination();
}

void Beam::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
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
	XMMATRIX T = XMMatrixTranslation( basePos.x, basePos.y, basePos.z );
	XMMATRIX W = S * R * T;
	XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr XMFLOAT4 lightDir{ 0.0f, -1.0f, 1.0f, 0.0f };
	
	pModel->Render( Float4x4( WVP ), Float4x4( W ), lightDir, color, cameraPosition, isEnableFill );

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		XMMATRIX colS = XMMatrixScaling( wsBody.size.x, wsBody.size.y, wsBody.size.z );

		Donya::Quaternion rotQ = Donya::Quaternion::Make( -Donya::Vector3::Right(), beamAngle );
		XMMATRIX colR = Matrix( rotQ.RequireRotationMatrix() );
		
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		// XMMATRIX colW = colS * colR * colT;
		XMMATRIX colW = colS * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 0.2f, 0.2f, 1.0f, 0.5f };

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

AABB Beam::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += beamDestPos;

	return wsHitBox;
}

bool Beam::ShouldErase() const
{
	return ( status == State::End ) ? true : false;
}

void Beam::SetPosture()
{
	posture = Donya::Quaternion::Make( -Donya::Vector3::Right(), beamAngle );
}

void Beam::AngleUpdate()
{
	using namespace Donya::Easing;
	float ease = Ease( scast<Kind>( easingKind ), scast<Type>( easingType ), easeParam );
	easeParam += angleIncreaseSpeed;

	beamAngle = beamAngleBegin + ( beamAngleEnd - beamAngleBegin ) * ease;

	if ( 1.0f <= easeParam )
	{
		beamAngle = beamAngleEnd;
		status = State::Wait;
	}

	SetPosture();
}

void Beam::WaitUpdate()
{
	afterWaitFrame--;
	if ( afterWaitFrame <= 0 )
	{
		status = State::End;
	}
}

void Beam::CalcBeamDestination()
{
	Donya::Vector3 beamVec3{};
	beamVec3.y = sinf( beamAngle ) * beamLength;
	beamVec3.z = cosf( beamAngle ) * beamLength;

	// These Vector2 is using to (Z,Y).

	Donya::Vector2 beamStart { basePos.z,  basePos.y  };
	Donya::Vector2 beamVec2  { beamVec3.z, beamVec3.y };

	float planeLength = beamLength * 2.0f;
	Donya::Vector2 planeStart{ beamStart.x - planeLength, 0.0f };
	Donya::Vector2 planeVec2 { planeLength * 2.0f, 0.0f };

	Donya::Vector2 intersection{};
	bool isIntersect = Common::CalcIntersectionPoint
	(
		planeStart, planeVec2,
		beamStart, beamVec2,
		&intersection
	);

	if ( isIntersect )
	{
		beamDestPos = Donya::Vector3{ basePos.x, intersection.y, intersection.x };
	}
	else
	{
		beamDestPos = Donya::Vector3::Zero();
		hitBox.exist = false;
	}
}

// region Beam
#pragma endregion

#pragma region Wave

Wave Wave::parameter{};
std::shared_ptr<Donya::StaticMesh> Wave::pModel{ nullptr };

void Wave::LoadModel()
{
	static bool wasLoaded = false;
	if ( wasLoaded ) { return; }
	// else

	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Wave ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load wave model." );

	pModel = Donya::StaticMesh::Create( loader );

	if ( !pModel )
	{
		_ASSERT_EXPR( 0, L"Failed : Load wave model." );
		exit( -1 );
	}

	wasLoaded = true;
}

void Wave::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, parameter );
}

#if USE_IMGUI

void Wave::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, parameter );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, parameter );
}

void Wave::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ウェーブ" ) )
		{
			ImGui::SliderInt( u8"消えるまでの時間（フレーム）", &parameter.aliveFrame, 1, 720 );
			ImGui::SliderFloat( u8"Ｚ軸の速度", &parameter.speed, 1.0f, 16.0f );
			ImGui::Text( "" );

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

Wave::Wave() :
	aliveFrame(),
	speed(),
	hitBox(),
	pos(),
	posture()
{

}
Wave::~Wave() = default;

void Wave::Init( const Donya::Vector3 &wsAppearPos )
{
	// Apply the external paramter.
	*this = parameter;

	pos = wsAppearPos;

	posture = Donya::Quaternion::Make( Donya::Vector3::Up(), ToRadian( 180.0f ) );
}
void Wave::Uninit()
{
	// No op.
}

void Wave::Update()
{
	aliveFrame--;
	// Emit Particles
	if (aliveFrame % 5 == 0)
	{
		ParticleManager::Get().CreateShockWaveParticle(pos);
	}

	pos.z -= speed; // The advance direction is back.
}

void Wave::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
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

		constexpr XMFLOAT4 colColor{ 1.0f, 0.1f, 0.0f, 0.5f };

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

AABB Wave::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += GetPos();
	return wsHitBox;
}

bool Wave::ShouldErase() const
{
	return ( aliveFrame <= 0 ) ? true : false;
}

// region Wave
#pragma endregion

#pragma region AttackParam

AttackParam::AttackParam() :
	maxHP( 1 ), counterMax(), untilAttackFrame(), resetWaitFrame(),
	damageWaitFrame(), stunFrame(),
	intervalsPerHP(), reuseFramesPerHP(),
	obstaclePatterns()
{
	intervalsPerHP.resize( maxHP );
	reuseFramesPerHP.resize( maxHP );
}
AttackParam::~AttackParam()
{
	obstaclePatterns.clear();
	obstaclePatterns.shrink_to_fit();
	intervalsPerHP.clear();
	intervalsPerHP.shrink_to_fit();
	reuseFramesPerHP.clear();
	reuseFramesPerHP.shrink_to_fit();
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
			int oldHP = maxHP;
			ImGui::SliderInt( u8"体力の最大値（１始まり）", &maxHP, 1, 32 );
			if ( oldHP != maxHP )
			{
				intervalsPerHP.resize( maxHP );
				reuseFramesPerHP.resize( maxHP );
			}
			ImGui::Text( "" );

			ImGui::SliderInt( u8"攻撃条件に使うカウンタの最大値", &counterMax, 100, 10240 );
			ImGui::SliderInt( u8"攻撃開始までの待機時間（フレーム）", &untilAttackFrame, 1, 2048 );
			ImGui::SliderInt( u8"リセット時の待機時間（フレーム）", &resetWaitFrame, 1, 2048 );
			ImGui::Text( "" );
			ImGui::SliderInt( u8"ダメージを受けた時の待機時間（フレーム）", &damageWaitFrame, 1, 2048 );
			ImGui::SliderInt( u8"ダメージを受けたあとの気絶時間（フレーム）", &stunFrame, 1, 2048 );
			ImGui::Text( "" );

			static int targetNo{};
			std::string attackNameU8{};
			switch ( targetNo )
			{
			case AttackKind::Missile:	attackNameU8 = u8"ミサイル";		break;
			case AttackKind::Obstacle:	attackNameU8 = u8"障害物";		break;
			case AttackKind::Beam:		attackNameU8 = u8"レーザー";		break;
			case AttackKind::Wave:		attackNameU8 = u8"ウェーブ";		break;
			default: attackNameU8 = u8"Error !"; break;
			}

			std::string caption{ u8"攻撃の種類：" };
			ImGui::SliderInt( ( caption + attackNameU8 ).c_str(), &targetNo, 0, AttackKind::ATTACK_KIND_COUNT - 1 );

			if ( ImGui::TreeNode( ( attackNameU8 + u8"・ＨＰ毎のパターン" ).c_str() ) )
			{
				for ( int i = maxHP - 1; 0 <= i; --i )
				{
					std::string strHP = "HP[" + std::to_string( i + 1 ) + "] : ";
					strHP = Donya::MultiToUTF8( strHP );

					const std::string STR_INTERVAL{ u8"攻撃の間隔（フレーム）" };
					const std::string STR_WAIT{ u8"攻撃後の待機時間（フレーム）" };

					ImGui::SliderInt( ( strHP + STR_INTERVAL ).c_str(), &intervalsPerHP[i][targetNo], 1, 2048 );
					ImGui::SliderInt( ( strHP + STR_WAIT ).c_str(), &reuseFramesPerHP[i][targetNo], 1, 2048 );
				}

				ImGui::TreePop();
			}

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

#pragma region CollisionDetail

CollisionDetail::CollisionDetail() :
	levelCount( LOWER_LEVEL_COUNT ),
	levelBorders()
{
	levelBorders.resize( LOWER_LEVEL_COUNT );
}
CollisionDetail::~CollisionDetail()
{
	levelBorders.clear();
	levelBorders.shrink_to_fit();
}

void CollisionDetail::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void CollisionDetail::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void CollisionDetail::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ボスの当たり判定の分割" ) )
		{
			int oldCount = levelCount;
			ImGui::SliderInt( u8"段階の数（１始まり）", &levelCount, LOWER_LEVEL_COUNT, 8 );
			if ( oldCount != levelCount )
			{
				levelBorders.resize( levelCount );
			}

			std::string preStrU8 { u8"威力[" };
			std::string postStrU8{ u8"]以上" };
			std::string numberU8{};
			std::string caption{};
			const int LEVEL_COUNT = scast<int>( levelBorders.size() );
			for ( int i = LEVEL_COUNT - 1; 0 <= i; --i ) // i is 0-based.
			{
				numberU8 = std::to_string( i + 1 ); // Showing number is 1-based.
				numberU8 = Donya::MultiToUTF8( numberU8 );

				caption = preStrU8 + numberU8 + postStrU8;

				ImGui::SliderFloat( caption.c_str(), &levelBorders[i], 0.0f, 1.0f );
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

// region CollisionDetail
#pragma endregion

#pragma region StunParam

StunParam::StunParam() :
	invisibleInterval(),
	rotationSpeedMiddle(),
	rotationSpeedHigh(),
	stunFrames(),
	stunVelocities()
{}
StunParam::~StunParam() = default;

void StunParam::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void StunParam::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void StunParam::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ボスの気絶関連" ) )
		{
			ImGui::SliderInt( u8"点滅間隔（フレーム）", &invisibleInterval, 1, 30 );
			ImGui::Text( "" );

			ImGui::SliderFloat( u8"回転速度（扇型）", &rotationSpeedMiddle, 0.0f, 32.0f );
			ImGui::SliderFloat( u8"回転速度（ぐるり）", &rotationSpeedHigh, 0.0f, 32.0f );
			
			if ( ImGui::TreeNode( u8"気絶時間の設定" ) )
			{
				std::string caption{};

				const size_t COUNT = stunFrames.size();
				for ( size_t i = 0; i < COUNT; ++i )
				{
					caption = "レベル：" + std::to_string( i );
					ImGui::SliderInt( Donya::MultiToUTF8( caption ).c_str(), &stunFrames[i], 1, 360 );
				}
				
				ImGui::TreePop();
			}
			if ( ImGui::TreeNode( u8"気絶時の移動速度の設定" ) )
			{
				std::string caption{};

				const size_t COUNT = stunVelocities.size();
				for ( size_t i = 0; i < COUNT; ++i )
				{
					caption = "レベル：" + std::to_string( i );
					ImGui::SliderFloat3( Donya::MultiToUTF8( caption ).c_str(), &stunVelocities[i].x, -64.0f, 64.0f );
				}
				
				ImGui::TreePop();
			}

			ImGui::Text( "" );

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

// region StunParam
#pragma endregion

#pragma region DestructionParam

DestructionParam::DestructionParam() :
	performanceFrame(),
	speedRange(),
	vectorPatterns()
{
	vectorPatterns.resize( 1U );
}
DestructionParam::~DestructionParam()
{
	vectorPatterns.clear();
	vectorPatterns.shrink_to_fit();
}

void DestructionParam::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void DestructionParam::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void DestructionParam::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ボスの壊れかた" ) )
		{
			ImGui::Text( u8"速さや方向は，指定した中からランダムで選ばれます" );
			ImGui::Text( u8"方向は，保存時に正規化されます" );
			ImGui::Text( "" );

			ImGui::SliderInt( u8"破壊演出の全体時間（フレーム）", &performanceFrame, 1, 360 );

			ImGui::SliderFloat2( u8"吹っ飛ぶ速さ（Min, Max）", &speedRange.x, 1.0f, 64.0f );

			if ( ImGui::TreeNode( u8"吹っ飛ぶ方向" ) )
			{
				if ( ImGui::Button( u8"パターンを増やす" ) )
				{
					vectorPatterns.push_back( {} );
				}
				if ( 1 < vectorPatterns.size() )
				{
					if ( ImGui::Button( u8"パターンを減らす" ) )
					{
						vectorPatterns.pop_back();
					}
				}
				ImGui::Text( "" );

				std::string caption{};
				const size_t COUNT = vectorPatterns.size();
				for ( size_t i = 0; i < COUNT; ++i )
				{
					caption = "[" + std::to_string( i ) + "]";
					ImGui::SliderFloat3( caption.c_str(), &vectorPatterns[i].x, -1.0f, 1.0f );

					std::string normalizeStrU8 = Donya::MultiToUTF8( caption + "：正規化" );
					if ( ImGui::Button( normalizeStrU8.c_str() ) )
					{
						vectorPatterns[i].Normalize();
					}
				}

				ImGui::TreePop();
			}
			ImGui::Text( "" );

			if ( ImGui::TreeNode( u8"ファイル" ) )
			{
				static bool isBinary = true;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "読み込み " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"保存" ) )
				{
					for ( auto &it : vectorPatterns )
					{
						it.Normalize();
					}

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

// region DestructionParam
#pragma endregion

#pragma region Boss

Boss::Boss() :
	status( State::Hidden ),
	currentHP( 1 ),
	attackTimer(), waitReuseFrame(),
	stunTimer(), bounceTimer(), destructTimer(),
	maxDistanceToTarget(), gravity(), initBouncePower(),
	hitBox(),
	bounceAppearTimeRange(), bouncePowerRange(),
	pos(), velocity(), stunVelocity(),
	missileOffset(), obstacleOffset(), waveOffset(),
	basePosture(),
	modelArm(), modelBody(), modelFoot(), modelRoll(), arm(),
	lanePositions(), missiles(), obstacles(), beams(), waves(),
	isLanding( true )
{}
Boss::~Boss()
{
	modelArm.pModel.reset();
	modelBody.pModel.reset();
	modelFoot.pModel.reset();
	modelRoll.pModel.reset();

	lanePositions.clear();
	lanePositions.shrink_to_fit();
	missiles.clear();
	missiles.shrink_to_fit();
	obstacles.clear();
	obstacles.shrink_to_fit();
	beams.clear();
	beams.shrink_to_fit();
	waves.clear();
	waves.shrink_to_fit();
}

void Boss::Init( float initDistanceFromOrigin, const std::vector<Donya::Vector3> &registerLanePositions )
{
	LoadParameter();
	LoadModel();

	Missile::LoadParameter();
	Missile::LoadModel();
	Obstacle::LoadParameter();
	Obstacle::LoadModel();
	Obstacle::Warning::LoadParameter();
	Obstacle::Warning::LoadSprite();
	Beam::LoadParameter();
	Beam::LoadModel();
	Wave::LoadParameter();
	Wave::LoadModel();

	AttackParam::Get().LoadParameter();
	CollisionDetail::Get().LoadParameter();
	DestructionParam::Get().LoadParameter();

	currentHP = AttackParam::Get().maxHP;

	waitReuseFrame = AttackParam::Get().untilAttackFrame;

	lanePositions = registerLanePositions;
	Obstacle::Warning::RegisterLaneCount( lanePositions.size() );

	basePosture = Donya::Quaternion::Make( Donya::Vector3::Up(), ToRadian( 180.0f ) );
}

void Boss::Uninit()
{
	for ( auto &it : missiles )
	{
		it.Uninit();
	}
	for ( auto &it : obstacles )
	{
		it.Uninit();
	}
	for ( auto &it : beams )
	{
		it.Uninit();
	}

	// Stop the looping sound.
	Donya::Sound::Stop( Music::BossEngine );
}

void Boss::StartUp( int targetLaneNo, float appearPositionZ )
{
	status = State::Normal;

	waitReuseFrame = AttackParam::Get().untilAttackFrame;

	pos = Donya::Vector3{ 0.0f, 0.0f, appearPositionZ };
	velocity.y = initBouncePower;
	isLanding = false;

	ShootMissile( targetLaneNo, /* setFastMode */ true );

	Donya::Sound::Play( Music::BossEngine );
}

void Boss::Update( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos )
{
#if USE_IMGUI

	UseImGui();
	Missile::UseImGui();
	Obstacle::UseImGui();
	Obstacle::Warning::UseImGui();
	Beam::UseImGui();
	Wave::UseImGui();
	AttackParam::Get().UseImGui();
	CollisionDetail::Get().UseImGui();
	DestructionParam::Get().UseImGui();

#endif // USE_IMGUI

	if ( status == State::Hidden ) { return; }
	// else

	UpdateCurrentStatus( targetLaneNo, wsAttackTargetPos );

	UpdateAttacks();
}

void Boss::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const DirectX::XMFLOAT4 &lightDirection, const DirectX::XMFLOAT4 &cameraPosition, bool isEnableFill ) const
{
	if ( status == State::Hidden ) { return; }
	if ( status == State::Dead   ) { return; }
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

	// Rendering model parts.
	{
		// Make matrix that is transform to parent-space for each model.
		/*
		Foot-space -> World-space.
		Body-space -> Foot-space.
		Arm-space  -> Body-space.
		Roll-space -> Arm-space.
		*/

		auto MakeModelMatrix = [&Matrix]( const ModelPart &mp )->XMMATRIX
		{
			XMMATRIX S = XMMatrixScaling( mp.scale.x, mp.scale.y, mp.scale.z );
			XMMATRIX R = Matrix( mp.posture.RequireRotationMatrix() );
			XMMATRIX T = XMMatrixTranslation( mp.offset.x, mp.offset.y, mp.offset.z );
			return S * R * T;
		};
		XMMATRIX MF = MakeModelMatrix( modelFoot );
		XMMATRIX MB = MakeModelMatrix( modelBody );
		XMMATRIX MA = MakeModelMatrix( modelArm  );
		XMMATRIX MR = MakeModelMatrix( modelRoll );

		auto MakeWorldMatrix = [&]()->XMMATRIX
		{
			// XMMATRIX S = XMMatrixIdentity();
			XMMATRIX R = Matrix( basePosture.RequireRotationMatrix() );
			XMMATRIX T = XMMatrixTranslation( pos.x, pos.y, pos.z );
			return R * T;
		};
		XMMATRIX W = MakeWorldMatrix();
	
		XMMATRIX WF = MF * W;  //                MF * W
		XMMATRIX WB = MB * WF; //           MB * MF * W
		XMMATRIX WA = MA * WB; //      MA * MB * MF * W
		XMMATRIX WR = MR * WA; // MR * MA * MB * MF * W

		auto RenderModelPart = [&]( const ModelPart &mp, const XMMATRIX &W )
		{
			XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

			constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

			mp.pModel->Render( Float4x4( WVP ), Float4x4( W ), lightDirection, color, cameraPosition, isEnableFill );
		};

		RenderModelPart( modelFoot, WF );
		RenderModelPart( modelBody, WB );
		RenderModelPart( modelArm,  WA );
		RenderModelPart( modelRoll, WR );
	}

	for ( const auto &it : missiles )
	{
		it.Draw( matView, matProjection, lightDirection, cameraPosition );
	}
	for ( const auto &it : obstacles )
	{
		it.Draw( matView, matProjection, lightDirection, cameraPosition );
	}
	for ( const auto &it : beams )
	{
		it.Draw( matView, matProjection, lightDirection, cameraPosition );
	}
	for ( const auto &it : waves )
	{
		it.Draw( matView, matProjection, lightDirection, cameraPosition );
	}

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();
		wsBody.size *= 2.0f;		// Use for scaling parameter. convert half-size to whole-size.

		const auto &LEVELS = CollisionDetail::Get().levelBorders;
		const size_t LEVEL_COUNT = LEVELS.size();

		auto MakeBodyPart = [&wsBody, &LEVELS, &LEVEL_COUNT]( size_t index )
		{
			float nextHeight = ( LEVEL_COUNT - 1 <= index ) ? 1.0f : LEVELS[index + 1];
			float heightPercent = ( nextHeight - LEVELS[index] );

			AABB wsPartBody = wsBody;
			wsPartBody.size.y *= heightPercent;

			float wsBottom = wsBody.pos.y - ( wsBody.size.y * 0.5f );
			float wsPartBottom = wsBottom + ( wsBody.size.y * LEVELS[index] );
			wsPartBody.pos.y = wsPartBottom + ( wsPartBody.size.y * 0.5f );

			return wsPartBody;
		};
		auto GetColor = [&LEVEL_COUNT]( size_t index )
		{
			constexpr std::array<XMFLOAT4, 5> COLORS
			{
				XMFLOAT4{ 1.0f, 0.4f, 0.3f, 0.5f },	// Red
				XMFLOAT4{ 0.4f, 0.6f, 1.0f, 0.5f },	// Blue
				XMFLOAT4{ 0.5f, 1.0f, 0.6f, 0.5f },	// Green
				XMFLOAT4{ 1.0f, 0.5f, 0.5f, 0.5f },	// Pink
				XMFLOAT4{ 0.8f, 0.8f, 0.8f, 0.5f },	// Gray
			};

			return ( COLORS.size() <= index )
			? XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.5f }	// White
			: COLORS[index];
		};

		auto InitializedCube = []()
		{
			Donya::Geometric::Cube cube{};
			cube.Init();
			return cube;
		};
		static Donya::Geometric::Cube cube = InitializedCube();

		for ( size_t i = 0; i < LEVEL_COUNT; ++i )
		{
			AABB wsBodyPart = MakeBodyPart( i );

			XMMATRIX S = XMMatrixScaling	( wsBodyPart.size.x, wsBodyPart.size.y, wsBodyPart.size.z );
			XMMATRIX T = XMMatrixTranslation( wsBodyPart.pos.x,  wsBodyPart.pos.y,  wsBodyPart.pos.z  );
			XMMATRIX W = S * T;

			XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

			cube.Render
			(
				Float4x4( WVP ),
				Float4x4( W ),
				lightDirection,
				GetColor( i )
			);
		}
	}

#endif // DEBUG_MODE
}

AABB Boss::GetHitBox() const
{
	if ( status == State::Hidden		) { return AABB::Nil(); }
	if ( status == State::Destruction	) { return AABB::Nil(); }
	if ( status == State::Dead			) { return AABB::Nil(); }
	// else

	AABB wsHitBox = hitBox;
	wsHitBox.pos += GetPos();
	return wsHitBox;
}

bool Boss::IsDead() const
{
	return ( status == State::Dead ) ? true : false;
}

const std::vector<Missile>  &Boss::FetchReflectableMissiles() const
{
	return missiles;
}
const std::vector<Obstacle> &Boss::FetchObstacles() const
{
	return obstacles;
}
std::vector<AABB> Boss::FetchHitBoxes() const
{
	const size_t BEAM_COUNT = beams.size();
	const size_t WAVE_COUNT = waves.size();
	std::vector<AABB> hitBoxes{};
	hitBoxes.reserve( BEAM_COUNT + WAVE_COUNT );

	for ( const auto &it : beams )
	{
		hitBoxes.emplace_back( it.GetHitBox() );
	}
	for ( const auto &it : waves )
	{
		hitBoxes.emplace_back( it.GetHitBox() );
	}

	return hitBoxes;
}

void Boss::ReceiveImpact( Donya::Vector3 wsCollidedPosition )
{
	AABB  wsHitBox  = GetHitBox();
	float bottomPos = std::max( 0.0f, wsHitBox.pos.y - wsHitBox.size.y );
	float hitBoxHeight = ( wsHitBox.pos.y + wsHitBox.size.y ) - bottomPos;
	if ( ZeroEqual( hitBoxHeight ) )
	{
		ReceiveDamage( 0 );
		return;
	}
	// else

	float normalizedOtherHeight = std::min( 1.0f, wsCollidedPosition.y / hitBoxHeight );
	if (  normalizedOtherHeight < 0.0f )
	{
		ReceiveDamage( 0 );
		return;
	}
	// else

	size_t damage = 0;

	auto &levels = CollisionDetail::Get().levelBorders;
	const int LEVEL_COUNT = scast<int>( levels.size() );
	for ( int i = LEVEL_COUNT - 1; 0 <= i; --i )
	{
		if ( levels[i] <= normalizedOtherHeight )
		{
			damage = i + 1; // To 1-based.
			break;
		}
	}

	ReceiveDamage( damage );
}

void Boss::LoadModel()
{
	constexpr size_t MODEL_COUNT = 4;
	const std::array<std::shared_ptr<Donya::StaticMesh> *, MODEL_COUNT> LOAD_MODELS
	{
		&modelArm.pModel,
		&modelBody.pModel,
		&modelFoot.pModel,
		&modelRoll.pModel
	};
	const std::array<std::string, MODEL_COUNT> LOAD_PATHS
	{
		GetModelPath( ModelAttribute::BossArm ),
		GetModelPath( ModelAttribute::BossBody ),
		GetModelPath( ModelAttribute::BossFoot ),
		GetModelPath( ModelAttribute::BossRoll )
	};

	for ( size_t i = 0; i < MODEL_COUNT; ++i )
	{
		Donya::Loader loader{};
		bool result = loader.Load( LOAD_PATHS[i], nullptr );

		_ASSERT_EXPR( result, L"Failed : Load boss model." );

		auto &loadModel = *( LOAD_MODELS[i] );
		loadModel = Donya::StaticMesh::Create( loader );

		_ASSERT_EXPR( loadModel, L"Failed : Load boss model." );
	}
}

void Boss::UpdateCurrentStatus( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos )
{
	auto PhysicalUpdate = [&]()
	{
		Move( wsAttackTargetPos );

		UpdateVertical();
	};

	switch ( status )
	{
	case Boss::State::Normal:

		PhysicalUpdate();

		LotteryAttack( targetLaneNo, wsAttackTargetPos );

		RotateRoll();

		break;
	case Boss::State::Stun:

		PhysicalUpdate();

		StunUpdate();

		break;
	case Boss::State::GenerateWave:

		PhysicalUpdate();

		ArmUpdate();

		RotateRoll();

		break;
	case Boss::State::Destruction:

		DestructionUpdate();

		break;

	case Boss::State::Dead:
		// No op.
		break;
	default:
		break;
	}
}

void Boss::RotateRoll()
{
	constexpr float SPEED = ToRadian( 12.0f );
	Donya::Quaternion rotation = Donya::Quaternion::Make( Donya::Vector3::Right(), SPEED );

	modelRoll.posture = rotation * modelRoll.posture;
}

void Boss::Move( const Donya::Vector3 &wsAttackTargetPos )
{
	// The position.y will changed at UpdateVertical().

	if ( IsStunning() )
	{
		pos.x += stunVelocity.x;
		pos.z += stunVelocity.z;
	}
	else
	{
		pos.x += velocity.x;
		pos.z += velocity.z;
	}

	float distance = wsAttackTargetPos.z - pos.z;
	if ( maxDistanceToTarget < fabsf( distance ) )
	{
		// Place to back than target-position.
		pos.z = wsAttackTargetPos.z + maxDistanceToTarget;
	}
}

void Boss::UpdateVertical()
{
	if ( !isLanding )
	{
		velocity.y -= gravity;
		pos.y += velocity.y;

		if ( pos.y < 0.0f )
		{
			pos.y		= 0.0f;
			velocity.y	= 0.0f;
			isLanding	= true;

			Donya::ScreenShake::SetY( Donya::ScreenShake::Kind::MOMENT, 2.0f, 2.0f, NULL, 0.03f );
			Donya::Sound::Play( Music::BossImpact );
		}
	}

	Bounce();
}
void Boss::Bounce()
{
	bool isAllowedBounce = ( status == State::Normal ) && ZeroEqual( pos.y );
	if ( isAllowedBounce )
	{
		bounceTimer--;

		if ( bounceTimer <= 0 )
		{
			float power = Donya::Random::GenerateFloat( bouncePowerRange.x, bouncePowerRange.y );
			velocity.y = power;

			float time = Donya::Random::GenerateFloat( bounceAppearTimeRange.x, bounceAppearTimeRange.y );
			bounceTimer = scast<int>( floorf( time ) );
		}
	}
}

void Boss::LotteryAttack( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos )
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
	for ( int i = 0; i < COUNT; ++i )	// This "i" is linking to enum of AttackParam::AttackKind.
	{
		auto &interval = PARAM.intervalsPerHP[currentHP - 1][i];
		if ( interval == 0 ) { continue; }
		// else

		if ( attackTimer % interval == 0 )
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
		case AttackParam::AttackKind::Missile:	ShootMissile( targetLaneNo );			break;
		case AttackParam::AttackKind::Obstacle:	GenerateObstacles( wsAttackTargetPos );	break;
		case AttackParam::AttackKind::Beam:		ShootBeam( targetLaneNo );				break;
		case AttackParam::AttackKind::Wave:		SetWaveMode();							break;
		default: break;
		}

		waitReuseFrame = PARAM.reuseFramesPerHP[currentHP - 1][useAttackNo];
	}

	if ( PARAM.counterMax <= attackTimer )
	{
		attackTimer = 0;
		waitReuseFrame += PARAM.resetWaitFrame;
	}
}

Donya::Vector3 Boss::LotteryLanePosition()
{
	const size_t laneCount = lanePositions.size();
	_ASSERT_EXPR( 0 < laneCount, L"The lane count is must over than zero !" );

	int index = Donya::Random::GenerateInt( 0, laneCount );
	return lanePositions[index];
}
Donya::Vector3 Boss::GetLanePosition( int laneNo )
{
	const int LANE_COUNT = scast<int>( lanePositions.size() ); // 1-based.
	_ASSERT_EXPR( laneNo < LANE_COUNT, L"Received lane-count is over than the actual lane-count !" );
	_ASSERT_EXPR( 0 < LANE_COUNT, L"The lane count is must over than zero !" );

	return lanePositions[laneNo];
}

void Boss::ShootMissile( int targetLaneNo, bool setFastMode )
{
	Donya::Vector3 appearPos = GetLanePosition( targetLaneNo );
	appearPos.z = pos.z;

	Donya::Vector3 dir = appearPos - pos;
	appearPos.x += missileOffset.x * Donya::SignBit( dir.x );
	appearPos.y += missileOffset.y;
	appearPos.z += missileOffset.z;

	missiles.push_back( {} );
	missiles.back().Init( appearPos, setFastMode );

	// Emit Particle
}
void Boss::UpdateMissiles()
{
	for ( auto &it : missiles )
	{
		it.Update( pos.z );
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

void Boss::GenerateObstacles( const Donya::Vector3 &wsAttackTargetPos )
{
	const auto &PATTERNS = AttackParam::Get().obstaclePatterns;
	const size_t PATTERN_COUNT = PATTERNS.size();
	if ( !PATTERN_COUNT ) { return; }
	// else

	const int RANDOM = Donya::Random::GenerateInt( 0, PATTERN_COUNT );
	
	auto Generate = [&]( size_t laneIndex )
	{
		Donya::Vector3 wsAppearPos = lanePositions[laneIndex];
		wsAppearPos.z += wsAttackTargetPos.z;

		Donya::Vector3 dir = wsAppearPos - pos;
		wsAppearPos.x += obstacleOffset.x * Donya::SignBit( dir.x );
		wsAppearPos.y += obstacleOffset.y;
		wsAppearPos.z += obstacleOffset.z;

		obstacles.push_back( {} );
		obstacles.back().Init( scast<int>( laneIndex ), wsAppearPos );
	};

	const auto &CHOSEN_LANE = PATTERNS[RANDOM];
	const size_t LANE_COUNT = CHOSEN_LANE.size();
	for ( size_t i = 0; i < LANE_COUNT; ++i )
	{
		if ( CHOSEN_LANE[i] != 0 )
		{
			// We assume the chosen-lane's size is same to the lane-position's size.
			Generate( i );
		}
	}
}
void Boss::UpdateObstacles()
{
	AABB wsBody = GetHitBox();
	AABB obstacleBox{};
	for ( auto &it : obstacles )
	{
		it.Update();

		obstacleBox = it.GetHitBox();
		if ( AABB::IsHitAABB( wsBody, obstacleBox, /* ignoreExistFlag = */ true ) )
		{
			it.HitToOther();
			Donya::Sound::Play( Music::ObjBreakObstacle );
		}
	}

	auto eraseItr = std::remove_if
	(
		obstacles.begin(), obstacles.end(),
		[]( Obstacle &element )
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
	obstacles.erase( eraseItr, obstacles.end() );
}

void Boss::ShootBeam( int targetLaneNo )
{
	Donya::Vector3 appearPos = GetLanePosition( targetLaneNo );
	appearPos.z = pos.z;

	Donya::Vector3 dir = appearPos - pos;
	appearPos.x += beamOffset.x * Donya::SignBit( dir.x );
	appearPos.y += beamOffset.y;
	appearPos.z += beamOffset.z;

	beams.push_back( {} );
	beams.back().Init( appearPos );
}
void Boss::UpdateBeams()
{
	for ( auto &it : beams )
	{
		it.Update( pos.z + beamOffset.z );
	}

	auto eraseItr = std::remove_if
	(
		beams.begin(), beams.end(),
		[]( Beam &element )
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
	beams.erase( eraseItr, beams.end() );
}

void Boss::SetWaveMode()
{
	status = State::GenerateWave;
	ResetArmState();
}
void Boss::ResetArmState()
{
	arm.status		= Arm::State::Rise;
	arm.easeParam	= 0.0f;
	arm.radian		= 0.0f;

	modelBody.posture = Donya::Quaternion::Identity();
}
void Boss::ArmUpdate()
{
	if ( arm.status == Arm::State::Vacation ) { return; }
	// else

	auto RiseUpdate = []( Arm &arm )
	{
		using namespace Donya::Easing;
		float ease = Ease( scast<Kind>( arm.easingKindRise ), Type::Out, arm.easeParam );
		arm.easeParam += arm.incrementation;

		arm.radian = arm.highestAngle * ease;

		if ( 1.0f <= arm.easeParam )
		{
			arm.status = Arm::State::Fall;

			arm.easeParam	= 0.0f;
			arm.radian		= arm.highestAngle;
		}
	};

	auto FallUpdate = []( Arm &arm )
	{
		using namespace Donya::Easing;
		float ease = Ease( scast<Kind>( arm.easingKindFall ), Type::In, arm.easeParam );
		arm.easeParam += arm.incrementation;

		arm.radian = arm.highestAngle - ( arm.highestAngle * ease );

		if ( 1.0f <= arm.easeParam )
		{
			arm.status = Arm::State::Vacation;

			arm.easeParam	= 0.0f;
			arm.radian		= 0.0f;

			Donya::ScreenShake::SetY
			(
				Donya::ScreenShake::Kind::MOMENT,
				arm.shakeStrength,
				arm.shakeDecel,
				NULL,
				arm.shakeInterval
			);
			Donya::Sound::Play( Music::BossImpact );
		}
	};

	switch ( arm.status )
	{
	case Arm::State::Fall:	FallUpdate( arm );	break;
	case Arm::State::Rise:	RiseUpdate( arm );	break;
	default: break;
	}

	float armPercent  = 1.0f - arm.bodyAngleRatio;
	float bodyPercent = arm.bodyAngleRatio;
	modelArm.posture  = Donya::Quaternion::Make( -Donya::Vector3::Right(), arm.radian * armPercent  );
	modelBody.posture = Donya::Quaternion::Make( -Donya::Vector3::Right(), arm.radian * bodyPercent );

	// If finished the wave omen.
	if ( arm.status == Arm::State::Vacation )
	{
		GenerateWave();

		ResetArmState();

		status = State::Normal;
	}
}
void Boss::GenerateWave()
{
	Donya::Vector3 appearPos = pos;
	appearPos.x = 0.0f;
	appearPos.y = 0.0f;

	appearPos += waveOffset;

	waves.push_back( {} );
	waves.back().Init( appearPos );

	Donya::Sound::Play( Music::BossRushWave );
}
void Boss::UpdateWaves()
{
	for ( auto &it : waves )
	{
		it.Update();
	}

	auto eraseItr = std::remove_if
	(
		waves.begin(), waves.end(),
		[]( Wave &element )
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
	waves.erase( eraseItr, waves.end() );
}

void Boss::UpdateAttacks()
{
	UpdateMissiles();
	UpdateObstacles();	// This method must call after we move(because doing collision-detection between myself and obstacle at it).
	UpdateBeams();
	UpdateWaves();
}

int  Boss::GetCurrentInjuryLevel() const
{
	// HACK:Doing hard coding... :(
	constexpr int LEVEL_COUNT = 3;
	constexpr std::array<int, LEVEL_COUNT> LEVEL_BORDERS
	{
		9,
		6,
		4
	};

	int i = 0;
	for ( ; i < LEVEL_COUNT; ++i )
	{
		if ( LEVEL_BORDERS[i] < currentHP )
		{
			break;
		}
	}
	return std::min( LEVEL_COUNT - 1, i );
}

void Boss::ReceiveDamage( int damage )
{
	if ( damage <= 0 ) { return; }
	// else

	const int &maxHP = AttackParam::Get().maxHP;
	if ( currentHP == maxHP )
	{
		ParticleManager::Get().StartBossDamageParticle();
	}

	const int oldInjuryLevel = GetCurrentInjuryLevel();

	currentHP -= damage;
	if ( currentHP <= 0 )
	{
		currentHP = 0;
		SetDestructMode();

		return;
	}
	// else

	const int currentInjuryLevel = GetCurrentInjuryLevel();
	if ( oldInjuryLevel != currentInjuryLevel )
	{
		ParticleManager::Get().UpdateBossDamageLevel();
	}

	status			= State::Stun;

	ResetArmState();

	auto &PARAM		= AttackParam::Get();
	waitReuseFrame	= PARAM.damageWaitFrame;
	stunTimer		= PARAM.stunFrame;

	Donya::Sound::Play( Music::BossReceiveDamage );
}
void Boss::StunUpdate()
{
	Donya::Quaternion rotation = Donya::Quaternion::Make( 0.0f, ToRadian( -16.0f ), 0.0f );
	basePosture = rotation * basePosture;

	stunTimer--;
	if ( stunTimer <= 0 )
	{
		stunTimer = 0;
		status = State::Normal;

		basePosture = Donya::Quaternion::Make( 0.0f, ToRadian( 180.0f ), 0.0f );
	}
}
bool Boss::IsStunning() const
{
	return ( status == State::Stun ) ? true : false;
}

void Boss::SetDestructMode()
{
	status = State::Destruction;

	// Erase obstacles.
	for ( auto &it : obstacles )
	{
		it.HitToOther();
	}

	// Set breaking process.
	{
		const auto &PARAM = DestructionParam::Get();
		destructTimer = PARAM.performanceFrame;

		constexpr size_t PART_COUNT = 3;
		const std::array<Donya::Vector3 *, PART_COUNT> VELOCITIES
		{
			&modelBody.velocity,
			&modelFoot.velocity,
			&modelRoll.velocity
		};

		const auto &PATTERNS = PARAM.vectorPatterns;
		const size_t PATTERN_COUNT = PATTERNS.size();
		float speed{};
		size_t patternIndex{};
		Donya::Vector3 vector{};
		for ( size_t i = 0; i < PART_COUNT; ++i )
		{
			using Random = Donya::Random;
			speed = Random::GenerateFloat( PARAM.speedRange.x, PARAM.speedRange.y ); // Min ~ Max
			patternIndex = Random::GenerateInt( 0, PATTERN_COUNT ); // Min ~ (Max - 1)

			vector = PATTERNS[patternIndex] * speed;
			*( VELOCITIES[i] ) = vector;
		}
	}
}
void Boss::DestructionUpdate()
{
	constexpr size_t PART_COUNT = 3;
	const std::array<ModelPart *, PART_COUNT> MODELS
	{
		&modelBody,
		&modelFoot,
		&modelRoll
	};

	for ( auto &it : MODELS )
	{
		it->velocity.y -= gravity;
		it->offset += it->velocity;
	}

	destructTimer--;
	if ( destructTimer <= 0 )
	{
		destructTimer = 0;
		status = State::Dead;
	}
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
			if ( ImGui::TreeNode( u8"パラメータ表示" ) )
			{
				ImGui::Text( u8"位置[X:%5.3f][Y:%5.3f][Z:%5.3f]", pos.x, pos.y, pos.z );
				ImGui::Text( u8"速度[X:%5.3f][Y:%5.3f][Z:%5.3f]", velocity.x, velocity.y, velocity.z );

				ImGui::TreePop();
			}

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
				ImGui::Text( u8"最大体力の設定は「攻撃パターン」欄にあります" );
				ImGui::Text( u8"ダメージを受けた際の気絶関連も「攻撃パターン」欄にあります" );
				ImGui::Text( "" );

				ImGui::SliderFloat3( u8"移動速度", &velocity.x, 0.1f, 32.0f );
				ImGui::SliderFloat3( u8"気絶中の移動速度", &stunVelocity.x, 0.1f, 32.0f );
				if ( 0.0f < velocity.z     ) { velocity.z     *= -1.0f; }
				if ( 0.0f < stunVelocity.z ) { stunVelocity.z *= -1.0f; }
				ImGui::Text( "" );

				ImGui::SliderFloat( u8"重力", &gravity, 0.0f, 16.0f );
				ImGui::SliderFloat( u8"登場時の跳ねる強さ", &initBouncePower, 1.0f, 512.0f );
				ImGui::SliderFloat2( u8"跳ねる間隔（フレーム）（Min, Max）", &bounceAppearTimeRange.x, 1.0f, 5120.0f );
				ImGui::SliderFloat2( u8"跳ねる強さ（Min, Max）", &bouncePowerRange.x, 1.0f, 512.0f );

				ImGui::SliderFloat( u8"プレイヤーとの距離の最大", &maxDistanceToTarget, 1.0f, 1024.0f );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"攻撃関連" ) )
			{
				ImGui::SliderFloat3( u8"ミサイル発射位置のオフセット（相対）", &missileOffset.x, -128.0f, 128.0f );
				ImGui::SliderFloat3( u8"障害物設置位置のオフセット（相対）", &obstacleOffset.x, -128.0f, 128.0f );
				ImGui::SliderFloat3( u8"ビーム設置位置のオフセット（相対）", &beamOffset.x, -128.0f, 128.0f );
				ImGui::SliderFloat3( u8"ウェーブ発生位置のオフセット（相対）", &waveOffset.x, -128.0f, 128.0f );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"モデル関連" ) )
			{
				auto ShowModelPartToGui = []( std::string name, ModelPart *pModel )
				{
					auto ToUTF8 = []( const std::string &str )
					{
						return Donya::MultiToUTF8( str );
					};

					ImGui::SliderFloat3( ToUTF8( name + "：位置のオフセット" ).c_str(), &pModel->offset.x, -128.0f, 128.0f );
					ImGui::SliderFloat3( ToUTF8( name + "：スケール" ).c_str(), &pModel->scale.x, 0.01f, 3.0f );
				};

				ShowModelPartToGui( "1:足もと",		&modelFoot );
				ShowModelPartToGui( "2:体",			&modelBody );
				ShowModelPartToGui( "3:腕",			&modelArm  );
				ShowModelPartToGui( "4:ローラー",	&modelRoll );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"腕関連（ウェーブ発生前兆）" ) )
			{
				static int wholeFrame =
				( ZeroEqual( arm.incrementation ) )
				? 2
				: scast<int>( 1.0f / arm.incrementation );
				ImGui::SliderInt( u8"前兆の全体時間（フレーム）", &wholeFrame, 2, 600 );
				arm.incrementation = 1.0f / scast<float>( wholeFrame );

				static float highestDegree = ToDegree( arm.highestAngle );
				ImGui::SliderFloat( u8"腕を振り上げる最大角度", &highestDegree, 0.0f, 360.0f );
				arm.highestAngle = ToRadian( highestDegree );

				ImGui::SliderFloat( u8"傾ける角度の内，体部分の割合", &arm.bodyAngleRatio, 0.0f, 1.0f );
				ImGui::Text( u8"体部分の最大角度：%05.3f", highestDegree * arm.bodyAngleRatio );
				ImGui::Text( u8"腕部分の最大角度：%05.3f", highestDegree * ( 1.0f - arm.bodyAngleRatio ) );
				ImGui::Text( "" );

				if ( ImGui::TreeNode( u8"画面シェイク" ) )
				{
					ImGui::SliderFloat( u8"強さ", &arm.shakeStrength, 0.01f, 64.0f );
					ImGui::SliderFloat( u8"揺れの減衰力", &arm.shakeDecel, 0.01f, 64.0f );
					ImGui::SliderFloat( u8"揺れる間隔", &arm.shakeInterval, 0.01f, 64.0f );

					ImGui::TreePop();
				}
				
				if ( ImGui::TreeNode( u8"イージングの種類" ) )
				{
					ImGui::SliderInt( u8"振り上げ・種類", &arm.easingKindRise, 0, GetEasingKindCount() - 1 );
					std::string easingCaption = "振り上げ・名：" + EasingKindToStr( arm.easingKindRise ) + " Out";
					easingCaption = Donya::MultiToUTF8( easingCaption );
					ImGui::Text( easingCaption.c_str() );

					ImGui::SliderInt( u8"振り降ろし・種類", &arm.easingKindFall, 0, GetEasingKindCount() - 1 );
					easingCaption = "振り降ろし・名：" + EasingKindToStr( arm.easingKindFall ) + " In";
					easingCaption = Donya::MultiToUTF8( easingCaption );
					ImGui::Text( easingCaption.c_str() );

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
