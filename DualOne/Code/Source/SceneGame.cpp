#include "SceneGame.h"

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "Donya/Constant.h"
#include "Donya/Easing.h"
#include "Donya/GamepadXInput.h"
#include "Donya/Keyboard.h"
#include "Donya/Mouse.h"
#include "Donya/Random.h"
#include "Donya/ScreenShake.h"
#include "Donya/Serializer.h"
#include "Donya/Sound.h"
#include "Donya/Sprite.h"
#include "Donya/Useful.h"
#include "Donya/UseImGui.h"

#include "Camera.h"
#include "Common.h"
#include "Effect.h"
#include "Fader.h"
#include "FilePath.h"
#include "Ground.h"
#include "Music.h"
#include "Player.h"
#include "StorageForScene.h"
#include "Timer.h"
#include "Boss.h"

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

// TODO:Swap this class to "UIObject" at UI.h.
struct Usage
{
public:
	size_t sprite{};
	float  degree{};
	float  alpha{};
	Donya::Vector2 ssPos{};	// screen-space.
	Donya::Vector2 scale{};
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( degree ),
			CEREAL_NVP( alpha ),
			CEREAL_NVP( ssPos ),
			CEREAL_NVP( scale )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
public:
	void LoadSprite()
	{
		sprite = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::Usage ), 4U );
	}

	void Draw()
	{
		Donya::Sprite::DrawExt
		(
			sprite,
			ssPos.x, ssPos.y,
			scale.x, scale.y,
			degree, alpha
		);
	}
};

CEREAL_CLASS_VERSION( Usage, 0 )

struct SceneGame::Impl
{
public:
	enum class State
	{
		Title,	// Showing the title-scene at another scene.
		Game,	// Playing main scene.
		Over,
	};
public:
	State	status;

	int		cameraEaseKind;			// Linking to Donya::Ease::Kind.
	int		cameraEaseType;			// Linking to Donya::Ease::Type.
	int		overCameraEaseKind;		// Linking to Donya::Ease::Kind.
	int		overCameraEaseType;		// Linking to Donya::Ease::Type.
	float	cameraLerpSpeed;		// 1.0f / (whole-frame).
	float	overCameraLerpSpeed;	// 1.0f / (whole-frame).

	float	cameraLerpFactor;		// 0.0f ~ 1.0f.
	float	overCameraLerpFactor;	// 0.0f ~ 1.0f.

	float	initDistanceOfBoss;		// Distance from origin.
	size_t	sprFont;

	Camera	camera;
	Player	player;
	Ground	ground;
	Boss	boss;
	Timer	currentTime;

	Usage	sprUsage;

	Donya::Vector3	lightDirection;

	Donya::Vector3	cameraDistance;		// Use at game-scene. X, Y is calculated from world-space, Z is calculated from local of player space.
	Donya::Vector3	cameraFocus;		// Use at game-scene. Relative position from the camera.
	Donya::Vector3	titleCameraDistance;// Use at title-scene. X, Y is calculated from world-space, Z is calculated from local of player space.
	Donya::Vector3	titleCameraFocus;	// Use at title-scene. Relative position from the camera.
	Donya::Vector3	overCameraDistance;	// Use at game-scene. X, Y is calculated from world-space, Z is calculated from local of player space.

	Donya::XInput	controller;

	std::vector<Donya::Vector3>		lanePositions;	// Only use when initialize.
	std::vector<ReflectedEntity>	reflectedEntities;

	bool	prepareStart;	// True when title-mode, and the player charging.
	bool	wasTouched;		// True when detect the collision between player and boss.
	bool	wasRetried;		// True if initialized from retry.
public:
	Impl() :
		status( State::Title ),
		cameraEaseKind(), cameraEaseType(), overCameraEaseKind(), overCameraEaseType(),
		cameraLerpSpeed(), overCameraLerpSpeed(), cameraLerpFactor(), overCameraLerpFactor(),
		initDistanceOfBoss(),
		sprFont( NULL ),
		camera(),
		player(),
		ground(),
		boss(),
		currentTime(),
		lightDirection( 0.0f, 0.0f, 1.0f ),
		cameraDistance( 0.0f, 1.0f, -1.0f ), cameraFocus( 0.0f, -0.5f, 1.0f ),
		titleCameraDistance( 0.0f, 1.0f, -1.0f ), titleCameraFocus( 0.0f, -0.5f, 1.0f ),
		overCameraDistance( 0.0f, 1.0f, -1.0f ),
		controller( Donya::Gamepad::PadNumber::PAD_1 ),
		lanePositions(), reflectedEntities(),
		prepareStart( false ), wasTouched( false ), wasRetried( false )
	{
		constexpr unsigned int DEFAULT_LANE_COUNT = 3;
		lanePositions.resize( DEFAULT_LANE_COUNT );
	}
	~Impl()
	{
		lanePositions.clear();
		lanePositions.shrink_to_fit();
		reflectedEntities.clear();
		reflectedEntities.shrink_to_fit();
	}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( lightDirection )
		);

		if ( 1 <= version )
		{
			archive
			(
				CEREAL_NVP( cameraDistance ),
				CEREAL_NVP( cameraFocus )
			);
		}
		if ( 2 <= version )
		{
			archive( CEREAL_NVP( initDistanceOfBoss ) );
		}
		if ( 3 <= version )
		{
			archive( CEREAL_NVP( lanePositions ) );
		}
		if ( 4 <= version )
		{
			archive
			(
				CEREAL_NVP( cameraEaseKind ),
				CEREAL_NVP( cameraEaseType ),
				CEREAL_NVP( cameraLerpSpeed ),
				CEREAL_NVP( titleCameraDistance ),
				CEREAL_NVP( titleCameraFocus )
			);
		}
		if ( 5 <= version )
		{
			archive( CEREAL_NVP( sprUsage ) );
		}
		if ( 6 <= version )
		{
			archive
			(
				CEREAL_NVP( overCameraEaseKind ),
				CEREAL_NVP( overCameraEaseType ),
				CEREAL_NVP( overCameraLerpSpeed ),
				CEREAL_NVP( overCameraDistance )
			);
		}
		if ( 7 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Game";
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
		Serializer::Extension bin  = Serializer::Extension::BINARY;
		Serializer::Extension json = Serializer::Extension::JSON;
		std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
		std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

		Serializer seria;
		seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
		seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
	}
	
	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( u8"一般設定" ) )
			{
				ImGui::Text( u8"Ｈキー：当たり判定の表示・非表示切り替え" );
				ImGui::Text( u8"Ｔキー：ImGuiの表示・非表示切り替え" );
				ImGui::Text( "" );

				ImGui::SliderFloat3( u8"ライトの方向", &lightDirection.x, -4.0f, 4.0f );
				ImGui::Text( "" );

				ImGui::SliderFloat3( u8"カメラの位置（自機からの相対）", &cameraDistance.x,	-1024.0f, 1024.0f );
				ImGui::SliderFloat3( u8"カメラ注視点（自身からの相対）", &cameraFocus.x,		-1024.0f, 1024.0f );
				ImGui::Text( "" );
				
				ImGui::SliderFloat3( u8"タイトル・カメラの位置（自機からの相対）", &titleCameraDistance.x,	-1024.0f, 1024.0f );
				ImGui::SliderFloat3( u8"タイトル・カメラ注視点（自身からの相対）", &titleCameraFocus.x,		-1024.0f, 1024.0f );
				ImGui::Text( "" );
				
				ImGui::SliderFloat3( u8"ゲームオーバー・カメラの移動先（自機からの相対）", &overCameraDistance.x,	-1024.0f, 1024.0f );
				ImGui::Text( "" );

				if ( ImGui::TreeNode( u8"カメラにかけるイージング" ) )
				{
					ImGui::SliderInt( u8"種類",		&cameraEaseKind, 0, GetEasingKindCount() - 1 );
					ImGui::SliderInt( u8"タイプ",	&cameraEaseType, 0, GetEasingTypeCount() - 1 );
					std::string easingCaption = "名：" + EasingKindToStr( cameraEaseKind ) + " " + EasingTypeToStr( cameraEaseType );
					easingCaption = Donya::MultiToUTF8( easingCaption );
					ImGui::Text( easingCaption.c_str() );
					
					ImGui::SliderInt( u8"オーバー・種類",		&overCameraEaseKind, 0, GetEasingKindCount() - 1 );
					ImGui::SliderInt( u8"オーバー・タイプ",	&overCameraEaseType, 0, GetEasingTypeCount() - 1 );
					easingCaption = "オーバー・名：" + EasingKindToStr( overCameraEaseKind ) + " " + EasingTypeToStr( overCameraEaseType );
					easingCaption = Donya::MultiToUTF8( easingCaption );
					ImGui::Text( easingCaption.c_str() );

					static int cameraMoveFrame =
					( ZeroEqual( cameraLerpSpeed ) )
					? 1
					: scast<int>( 1.0f / cameraLerpSpeed );
					ImGui::SliderInt( u8"移動にかける時間（フレーム）", &cameraMoveFrame, 1, 256 );
					cameraLerpSpeed = 1.0f / scast<float>( cameraMoveFrame );

					static int overCameraMoveFrame =
					( ZeroEqual( overCameraLerpSpeed ) )
					? 1
					: scast<int>( 1.0f / overCameraLerpSpeed );
					ImGui::SliderInt( u8"オーバー・移動にかける時間（フレーム）", &overCameraMoveFrame, 1, 1024 );
					overCameraLerpSpeed = 1.0f / scast<float>( overCameraMoveFrame );

					ImGui::TreePop();
				}
				ImGui::Text( "" );

				ImGui::SliderFloat( u8"初期のボスとの距離", &initDistanceOfBoss, 0.01f, 512.0f );
				ImGui::Text( "" );

				if ( ImGui::TreeNode( u8"レーンの設定" ) )
				{
					ImGui::Text( u8"０始まり，左から数える。" );
					ImGui::Text( u8"変更はゲームシーン初期化時に反映されます。" );
					ImGui::Text( "" );
					
					if ( ImGui::Button( u8"レーンを増やす" ) )
					{
						lanePositions.push_back( {} );
					}
					if ( !lanePositions.empty() )
					{
						if ( ImGui::Button( u8"レーンを減らす" ) )
						{
							lanePositions.pop_back();
						}
					}
					ImGui::Text( "" );

					const size_t COUNT = lanePositions.size();
					for ( size_t i = 0; i < COUNT; ++i )
					{
						std::string captionU8 = "レーン[" + std::to_string( i ) + "]・座標";
						captionU8 = Donya::MultiToUTF8( captionU8 );
						ImGui::SliderFloat3( captionU8.c_str(), &lanePositions[i].x, -256.0f, 256.0f );
					}

					ImGui::TreePop();
				}

				if ( ImGui::TreeNode( u8"操作方法の表示位置" ) )
				{
					ImGui::SliderFloat( u8"角度", &sprUsage.degree, 0.0f, 360.0f );
					ImGui::SliderFloat( u8"アルファ", &sprUsage.alpha, 0.0f, 1.0f );
					ImGui::SliderFloat2( u8"描画位置（Ｘ，Ｙ）", &sprUsage.ssPos.x, 0.0f, 1920.0f );
					ImGui::SliderFloat2( u8"スケール（Ｘ，Ｙ）", &sprUsage.scale.x, 0.0f, 1.0f );

					ImGui::TreePop();
				}

				if ( ImGui::TreeNode( u8"ファイル" ) )
				{
					static bool isBinary = false;
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

			camera.ShowParametersToImGui();

			ImGui::End();
		}
	}

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( SceneGame::Impl, 6 )

SceneGame::SceneGame() : pImpl( std::make_unique<Impl>() )
{

}
SceneGame::~SceneGame()
{
	pImpl.reset( nullptr );
}

void SceneGame::Init()
{
	pImpl->LoadParameter();

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::Font ), 1024U );
	pImpl->sprUsage.LoadSprite();

	pImpl->currentTime.Set( 0, 0, 0 );

	pImpl->ground.Init();

	pImpl->player.Init( pImpl->lanePositions );

	pImpl->boss.Init( pImpl->initDistanceOfBoss, pImpl->lanePositions );

	ParticleManager::Get().Init();

	// The camera's initialize should call after player's initialize.
	{
		Donya::Vector3 cameraPosition = pImpl->titleCameraDistance;
		cameraPosition.z += pImpl->player.GetPos().z;

		constexpr float FOV = ToRadian( 30.0f );
		pImpl->camera.Init( Common::ScreenWidthF(), Common::ScreenHeightF(), FOV );
		pImpl->camera.SetFocusCoordinate( pImpl->player.GetPos() + pImpl->titleCameraFocus );
		pImpl->camera.SetPosition( cameraPosition );
	}

	pImpl->wasRetried = StorageForScene::Get().GetRetryFlag();
	StorageForScene::Get().StoreRetryFlag( false );
}

void SceneGame::Uninit()
{
	ParticleManager::Get().Uninit();

	pImpl->player.Uninit();

	for ( auto &it : pImpl->reflectedEntities )
	{
		it.Uninit();
	}

	pImpl->boss.Uninit();

	pImpl->ground.Uninit();

	Donya::ScreenShake::StopX();
	Donya::ScreenShake::StopY();

	Donya::Sound::Stop( Music::BGM_Game );
}

Scene::Result SceneGame::Update( float elapsedTime )
{
	pImpl->controller.Update();

	auto MakePlayerInput = [&]()->Player::Input
	{
		Player::Input input{};

		auto &ctrller = pImpl->controller;
		if ( ctrller.IsConnected() )
		{
			bool triggerLeft  = ctrller.Trigger( Donya::Gamepad::Button::LEFT  ) || ctrller.TriggerStick( Donya::Gamepad::StickDirection::LEFT  );
			bool triggerRight = ctrller.Trigger( Donya::Gamepad::Button::RIGHT ) || ctrller.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
			if ( triggerLeft  ) { input.stick.x = -1.0f; }
			if ( triggerRight ) { input.stick.x =  1.0f; }

			if ( ctrller.Press( Donya::Gamepad::Button::A ) ) { input.doCharge = true; }
		}
		else
		{
			if ( Donya::Keyboard::Trigger( VK_RIGHT ) ) { input.stick.x =  1.0f; }
			if ( Donya::Keyboard::Trigger( VK_LEFT  ) ) { input.stick.x = -1.0f; }

			if ( Donya::Keyboard::Press( 'Z' ) ) { input.doCharge = true; }
		}

		if ( pImpl->status == Impl::State::Title )
		{
			input.stick = 0.0f;
		}

		return input;
	};
	Player::Input playerInput{};

	if ( pImpl->status == Impl::State::Title )
	{
		if ( IsDecisionReleased() || pImpl->wasRetried )
		{
			pImpl->prepareStart = true;
		}
	}
	else if ( !pImpl->wasTouched ) // Not failed.
	{
		pImpl->currentTime.Update();
	}

	playerInput = MakePlayerInput();

	if ( pImpl->prepareStart )
	{
		// Charging player for reflect a missile, must do this before change the status.
		if ( !pImpl->player.IsFullCharged() )
		{
			playerInput.doCharge = true;
		}
		else if ( !playerInput.doCharge )
		{
			pImpl->status		= Impl::State::Game;
			pImpl->prepareStart	= false;

			pImpl->boss.StartUp
			(
				pImpl->player.GetCurrentLane(),
				pImpl->player.GetPos().z + pImpl->initDistanceOfBoss
			);

			Donya::Sound::Play( Music::BGM_Game );
		}
	}

#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->ground.Update( pImpl->player.GetPos() );

	if ( !pImpl->wasTouched )
	{
		pImpl->player.Update( playerInput );
	}

	// Update "pImpl->reflectedEntities".
	{
		for ( auto &it : pImpl->reflectedEntities )
		{
			it.Update();
		}

		auto result = std::remove_if
		(
			pImpl->reflectedEntities.begin(),
			pImpl->reflectedEntities.end(),
			[]( ReflectedEntity &element )
			{
				if ( element.ShouldErase() )
				{
					ParticleManager::Get().ReserveExplosionParticles( element.GetPos(), 120, 1 );
					return true;
				}
				// else
				return false;
			}
		);

		pImpl->reflectedEntities.erase( result, pImpl->reflectedEntities.end() );
	}

	pImpl->boss.Update( pImpl->player.GetCurrentLane(), pImpl->player.GetPos() );

	// Update Particles.
	{
		ParticleEmitterPosition arg;
		arg.playerPos = pImpl->player.GetPos();
		arg.bossPos = pImpl->boss.GetPos();
		ParticleManager::Get().Update( arg );
	}

	UpdateCamera();

	DetectCollision();

#if DEBUG_MODE

	if ( Donya::Keyboard::Press( 'A' ) )
	{
		bool enableImpact = false;
		Donya::Vector3 tmpCollidePos{};
		if ( Donya::Keyboard::Trigger( '1' ) ) { tmpCollidePos.y =  0.0f; enableImpact = true; }
		if ( Donya::Keyboard::Trigger( '2' ) ) { tmpCollidePos.y = 50.0f; enableImpact = true; }
		if ( Donya::Keyboard::Trigger( '3' ) ) { tmpCollidePos.y =999.0f; enableImpact = true; }

		if ( enableImpact )
		{
			pImpl->boss.ReceiveImpact( tmpCollidePos );
		}
	}
	if ( Donya::Keyboard::Trigger( 'F' ) )
	{
		ParticleManager::Get().ReserveExplosionParticles( pImpl->player.GetPos(), 1200, 1 );
	}

#endif // DEBUG_MODE

	return ReturnResult();
}

void SceneGame::Draw( float elapsedTime )
{
	// Drawing 2D asset.
	{
		Donya::Sprite::SetDrawDepth( 1.0f );

		// BackGround.
		Donya::Sprite::DrawRect
		(
			Common::HalfScreenWidthF(),
			Common::HalfScreenHeightF(),
			Common::ScreenWidthF(),
			Common::ScreenHeightF(),
			Donya::Sprite::Color::DARK_GRAY, 1.0f
		);

		Donya::Sprite::SetDrawDepth( 0.0f );

		pImpl->sprUsage.Draw();

		if ( pImpl->status != Impl::State::Title )
		{
			Donya::Sprite::DrawString
			(
				pImpl->sprFont,
				pImpl->currentTime.ToStr(),
				32.0f, 64.0f,
				64.0f, 64.0f,
				64.0f, 64.0f
			);
		}
	}

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
	auto ToFloat4	= []( const Donya::Vector3 &vec3, float fourthValue )
	{
		return Donya::Vector4{ vec3.x, vec3.y, vec3.z, fourthValue };
	};

	XMFLOAT4X4 matView = Float4x4( pImpl->camera.CalcViewMatrix() );
	XMFLOAT4X4 matProj = Float4x4( pImpl->camera.GetProjectionMatrix() );
	Donya::Vector4 lightDir  = ToFloat4( pImpl->lightDirection, 0.0f );
	Donya::Vector4 cameraPos = ToFloat4( pImpl->camera.GetPos(), 1.0f );

	pImpl->ground.Draw( matView, matProj, lightDir, cameraPos );

	pImpl->player.Draw( matView, matProj, lightDir, cameraPos );

	pImpl->boss.Draw( matView, matProj, lightDir, cameraPos );

	ParticleManager::Get().Draw( matView, matProj, lightDir, cameraPos );

	for ( const auto &it : pImpl->reflectedEntities )
	{
		it.Draw( matView, matProj, lightDir, cameraPos );
	}
}

bool SceneGame::IsDecisionTriggered() const
{
	return
	( pImpl->controller.IsConnected() )
	? pImpl->controller.Trigger( Donya::Gamepad::A )
	: ( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false;
}
bool SceneGame::IsDecisionReleased() const
{
	return
	( pImpl->controller.IsConnected() )
	? pImpl->controller.Release( Donya::Gamepad::A )
	: ( Donya::Keyboard::Release( 'Z' ) ) ? true : false;
}

bool SceneGame::IsDoneCameraMove() const
{
	return ( pImpl->cameraLerpFactor < 1.0f ) ? false : true;
}

void SceneGame::UpdateCamera()
{
	Camera::Controller cameraController{};
	cameraController.SetNoOperation();

#if DEBUG_MODE
	auto MakeControlStructWithMouse = []()
	{
		static Donya::Int2 prevMouse{};
		static Donya::Int2 currMouse{};

		prevMouse = currMouse;

		auto nowMouse = Donya::Mouse::Coordinate();
		currMouse.x = scast<int>( nowMouse.x );
		currMouse.y = scast<int>( nowMouse.y );

		bool isInputMouseButton = Donya::Mouse::Press( Donya::Mouse::Kind::LEFT ) || Donya::Mouse::Press( Donya::Mouse::Kind::MIDDLE ) || Donya::Mouse::Press( Donya::Mouse::Kind::RIGHT );
		bool isDriveMouse = ( prevMouse != currMouse ) || Donya::Mouse::WheelRot() || isInputMouseButton;
		if ( !isDriveMouse || Donya::IsMouseHoveringImGuiWindow() )
		{
			Camera::Controller noop{};
			noop.SetNoOperation();
			return noop;
		}

		Donya::Vector3 diff{};
		{
			Donya::Vector2 vec2 = ( currMouse - prevMouse ).Float();

			diff.x = vec2.x;
			diff.y = vec2.y * -1.0f;
		}

		Donya::Vector3 movement{};
		Donya::Vector3 rotation{};

		if ( Donya::Mouse::Press( Donya::Mouse::Kind::LEFT ) )
		{
			constexpr float ROT_AMOUNT = ToRadian( 1.0f );
			rotation.x = diff.x * ROT_AMOUNT;
			rotation.y = diff.y * ROT_AMOUNT;
		}
		else
		if ( Donya::Mouse::Press( Donya::Mouse::Kind::MIDDLE ) )
		{
			constexpr float MOVE_SPEED = 0.1f;
			movement.x = diff.x * MOVE_SPEED;
			movement.y = diff.y * MOVE_SPEED;
		}

		constexpr float FRONT_SPEED = 3.5f;
		movement.z = FRONT_SPEED * scast<float>( Donya::Mouse::WheelRot() );

		Donya::Quaternion rotYaw = Donya::Quaternion::Make( Donya::Vector3::Up(), rotation.x );

		Donya::Vector3 right = Donya::Vector3::Right();
		right = rotYaw.RotateVector( right );
		Donya::Quaternion rotPitch = Donya::Quaternion::Make( right, rotation.y );

		Donya::Quaternion rotQ = rotYaw * rotPitch;

		static Donya::Vector3 front = Donya::Vector3::Front();

		if ( !rotation.IsZero() )
		{
			front = rotQ.RotateVector( front );
			front.Normalize();
		}

		Camera::Controller ctrl{};
		ctrl.moveVelocity = movement;
		ctrl.rotation = rotation;
		ctrl.slerpPercent = 1.0f;
		ctrl.moveAtLocalSpace = true;

		return ctrl;
	};
	// cameraController = MakeControlStructWithMouse();
#endif // DEBUG_MODE

	pImpl->camera.Update( cameraController );

	Donya::Vector3 nowCameraDistance	= pImpl->titleCameraDistance;
	Donya::Vector3 nowCameraFocus		= pImpl->titleCameraFocus;
	if ( pImpl->status != Impl::State::Title )
	{
		if ( !IsDoneCameraMove() )
		{
			Donya::Easing::Kind kind = scast<Donya::Easing::Kind>( pImpl->cameraEaseKind );
			Donya::Easing::Type type = scast<Donya::Easing::Type>( pImpl->cameraEaseType );
			float ease = Ease( kind, type, pImpl->cameraLerpFactor );

			Donya::Vector3 distVec  = pImpl->cameraDistance	- pImpl->titleCameraDistance;
			Donya::Vector3 focusVec = pImpl->cameraFocus	- pImpl->titleCameraFocus;
			distVec  *= ease;
			focusVec *= ease;
			nowCameraDistance	+= distVec;
			nowCameraFocus		+= focusVec;

			pImpl->cameraLerpFactor += pImpl->cameraLerpSpeed;
			pImpl->cameraLerpFactor = std::min( 1.0f, pImpl->cameraLerpFactor );
		}
		else
		{
			if ( pImpl->wasTouched )
			{
				nowCameraDistance	= pImpl->cameraDistance;
				nowCameraFocus		= pImpl->cameraFocus;

				Donya::Easing::Kind kind = scast<Donya::Easing::Kind>( pImpl->overCameraEaseKind );
				Donya::Easing::Type type = scast<Donya::Easing::Type>( pImpl->overCameraEaseType );
				float ease = Ease( kind, type, pImpl->overCameraLerpFactor );

				Donya::Vector3 destination = pImpl->overCameraDistance;

				Donya::Vector3 vecToDest = destination - nowCameraFocus;
				vecToDest *= ease;

				nowCameraDistance += vecToDest;

				pImpl->overCameraLerpFactor += pImpl->overCameraLerpSpeed;
				pImpl->overCameraLerpFactor = std::min( 1.0f, pImpl->overCameraLerpFactor );

				if ( 1.0f <= pImpl->overCameraLerpFactor )
				{
					pImpl->status = Impl::State::Over;
				}
			}
			else
			{
				nowCameraDistance	= pImpl->cameraDistance;
				nowCameraFocus		= pImpl->cameraFocus;
			}
		}
	}

	Donya::Vector3 criteria = pImpl->player.GetPos();
	criteria.x = criteria.y = 0.0f;
	pImpl->camera.SetPosition( criteria + nowCameraDistance );
	pImpl->camera.SetFocusCoordinate( criteria + nowCameraFocus );
}

void SceneGame::DetectCollision()
{
	auto HitToPlayer = [&]( bool canReflection )
	{
		auto result = pImpl->player.ReceiveImpact( canReflection );
		if ( result.shouldGenerateBullet )
		{
			pImpl->reflectedEntities.emplace_back();
			pImpl->reflectedEntities.back().Init
			(
				result.gravity,
				result.hitBox,
				result.wsPos,
				result.velocity
			);
		}
	};

	const AABB playerBox = pImpl->player.GetHitBox();

	// Missiles vs Player.
	{
		AABB other{};
		auto &reflectableAttacks = pImpl->boss.FetchReflectableMissiles();
		for ( const auto &it : reflectableAttacks )
		{
			other = it.GetHitBox();
			if ( AABB::IsHitAABB( other, playerBox, /* ignoreExistFlag = */ true ) )
			{
				it.HitToOther();

				HitToPlayer( /* canReflection = */ true );

				ParticleManager::Get().ReserveExplosionParticles( it.GetPos(), 120, 1 );
			}
		}
	}
	// Obstacles vs Player.
	{
		AABB other{};
		auto &obstacles = pImpl->boss.FetchObstacles();
		for ( const auto &it : obstacles )
		{
			other = it.GetHitBox();
			if ( AABB::IsHitAABB( other, playerBox ) )
			{
				it.HitToOther();

				HitToPlayer( /* canReflection = */ false );
			}
		}
	}
	// Beams vs Player.
	{
		AABB other{};
		auto &beams = pImpl->boss.FetchBeams();
		for ( const auto &it : beams )
		{
			other = it.GetHitBox();
			if ( AABB::IsHitAABB( other, playerBox ) )
			{
				it.HitToOther();

				HitToPlayer( /* canReflection = */ false );
			}
		}
	}
	// Other-Attacks vs Player.
	{
		std::vector<AABB> hitBoxes = pImpl->boss.FetchHitBoxes();
		for ( const auto &it : hitBoxes )
		{
			if ( AABB::IsHitAABB( it, playerBox ) )
			{
				HitToPlayer( /* canReflection = */ false );
			}
		}
	}

	// Reflected-Entity vs Boss.
	{
		const AABB boss = pImpl->boss.GetHitBox();
		Sphere entity{};
		for ( const auto &it : pImpl->reflectedEntities )
		{
			entity = it.GetHitBox();
			if ( AABB::IsHitSphere( boss, entity ) )
			{
				it.HitToOther();

				pImpl->boss.ReceiveImpact( entity.pos );
			}
		}
	}

	// Player vs Boss.
	{
		const AABB boss = pImpl->boss.GetHitBox();

		if ( AABB::IsHitAABB( playerBox, boss ) )
		{
			pImpl->wasTouched = true;
		}
	}
}

Scene::Result SceneGame::ReturnResult()
{
	if ( Fader::Get().IsClosed() )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Title;
		return change;
	}
	// else

	bool requestPause = pImpl->controller.Trigger( Donya::Gamepad::Button::START ) || pImpl->controller.Trigger( Donya::Gamepad::Button::SELECT ) || Donya::Keyboard::Trigger( 'P' );
	bool allowPause   = ( pImpl->status == Impl::State::Game && !Fader::Get().IsExist() );
	if ( requestPause && allowPause )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result pause{};
		pause.AddRequest( Scene::Request::ADD_SCENE );
		pause.sceneType = Scene::Type::Pause;
		return pause;
	}
	// else

	if ( pImpl->status == Impl::State::Over )
	{
		Donya::Sound::Stop( Music::BGM_Game );		// Game scene is not erased for showing scene of clear, so I should stop the BGM here.

		StorageForScene::Get().StoreTimer( pImpl->currentTime );

		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE );
		change.sceneType = Scene::Type::Over;
		return change;
	}

#if DEBUG_MODE
	if ( ( Donya::Keyboard::Trigger( VK_RETURN ) && Donya::Keyboard::Press( VK_RSHIFT ) ) || pImpl->boss.IsDead() )
	{
		Donya::Sound::Play( Music::ItemDecision );
		Donya::Sound::Stop( Music::BGM_Game );		// Game scene is not erased for showing scene of clear, so I should stop the BGM here.

		StorageForScene::Get().StoreTimer( pImpl->currentTime );

		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE );
		change.sceneType = Scene::Type::Clear;
		return change;
	}
	// else
	if ( Donya::Keyboard::Trigger( 'Q' ) )
	{
		Donya::Sound::Play( Music::ItemDecision );
		Donya::Sound::Stop( Music::BGM_Game );		// Game scene is not erased for showing scene of clear, so I should stop the BGM here.

		StorageForScene::Get().StoreTimer( pImpl->currentTime );

		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE );
		change.sceneType = Scene::Type::Over;
		return change;
	}
	// else
#endif // DEBUG_MODE

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}