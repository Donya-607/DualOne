#include "SceneGame.h"

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "Donya/Constant.h"
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
#include "Fader.h"
#include "FilePath.h"
#include "Ground.h"
#include "Music.h"
#include "Player.h"
#include "StorageForScene.h"
#include "Timer.h"
#include "Boss.h"
#include "Effect.h"

struct SceneGame::Impl
{
public:
	float	initDistanceOfBoss;		// Distance from origin.
	size_t	sprFont;
	Camera	camera;
	Player	player;
	Ground	ground;
	Boss	boss;
	ParticleManager particleManager;
	Timer	currentTime;
	Donya::Vector3	lightDirection;
	Donya::Vector3	cameraDistance;	// X, Y is calculated from world-space, Z is calculated from local of player space.
	Donya::Vector3	cameraFocus;	// Relative position from the camera.
	Donya::XInput	controller;
	std::vector<Donya::Vector3>		lanePositions;	// Only use when initialize.
	std::vector<ReflectedEntity>	reflectedEntities;
public:
	Impl() :
		initDistanceOfBoss(),
		sprFont( NULL ),
		camera(),
		player(),
		ground(),
		boss(),
		currentTime(),
		lightDirection( 0.0f, 0.0f, 1.0f ),
		cameraDistance( 0.0f, 1.0f, -1.0f ), cameraFocus( 0.0f, -0.5f, 1.0f ),
		controller( Donya::Gamepad::PadNumber::PAD_1 ),
		lanePositions(), reflectedEntities()
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
			/*
			archive( CEREAL_NVP() );
			*/
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
			if ( ImGui::TreeNode( u8"一般設定" ) )
			{
				ImGui::SliderFloat3( u8"ライトの方向", &lightDirection.x, -4.0f, 4.0f );
				ImGui::Text( "" );

				ImGui::SliderFloat3( u8"カメラの位置（自機からの相対）", &cameraDistance.x, -512.0f, 512.0f );
				ImGui::SliderFloat3( u8"カメラ注視点（自身からの相対）", &cameraFocus.x, -512.0f, 512.0f );
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

CEREAL_CLASS_VERSION( SceneGame::Impl, 3 )

SceneGame::SceneGame() : pImpl( std::make_unique<Impl>() )
{

}
SceneGame::~SceneGame()
{
	pImpl.reset( nullptr );
}

void SceneGame::Init()
{
	Donya::Sound::Play( Music::BGM_Game );

	pImpl->LoadParameter();

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::TestFont ), 1024U );

	pImpl->currentTime.Set( 0, 0, 0 );

	pImpl->ground.Init();

	pImpl->player.Init( pImpl->lanePositions );

	pImpl->boss.Init( pImpl->initDistanceOfBoss, pImpl->lanePositions );

	pImpl->particleManager.Init();

	// The camera's initialize should call after player's initialize.
	constexpr float FOV = ToRadian( 30.0f );
	pImpl->camera.Init( Common::ScreenWidthF(), Common::ScreenHeightF(), FOV );
	pImpl->camera.SetFocusCoordinate( pImpl->player.GetPos() + pImpl->cameraFocus );
	pImpl->camera.SetPosition( pImpl->player.GetPos() + pImpl->cameraDistance );
}

void SceneGame::Uninit()
{
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

	pImpl->currentTime.Update();

#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->ground.Update(pImpl->player.GetPos());

	auto MakePlayerInput = [&]()->Player::Input
	{
		Player::Input input{};

		auto &ctrller = pImpl->controller;
		if ( ctrller.IsConnected() )
		{
			if ( ctrller.Trigger( Donya::Gamepad::Button::LEFT  ) ) { input.stick.x =  1.0f; }
			if ( ctrller.Trigger( Donya::Gamepad::Button::RIGHT ) ) { input.stick.x = -1.0f; }

			if ( ctrller.Press( Donya::Gamepad::Button::A ) ) { input.doCharge = true; }
		}
		else
		{
			if ( Donya::Keyboard::Trigger( VK_RIGHT ) ) { input.stick.x =  1.0f; }
			if ( Donya::Keyboard::Trigger( VK_LEFT  ) ) { input.stick.x = -1.0f; }

			if ( Donya::Keyboard::Press( 'Z' ) ) { input.doCharge = true; }
		}

		return input;
	};
	pImpl->player.Update( MakePlayerInput() );

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
				return element.ShouldErase();
			}
		);

		pImpl->reflectedEntities.erase( result, pImpl->reflectedEntities.end() );
	}

	pImpl->boss.Update( pImpl->player.GetPos() );
	pImpl->particleManager.Update();

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

	Donya::Vector3 criteria = pImpl->player.GetPos();
	criteria.x = criteria.y = 0.0f;
	pImpl->camera.SetPosition( criteria + pImpl->cameraDistance );
	pImpl->camera.SetFocusCoordinate( criteria + pImpl->cameraFocus );

	DetectCollision();

	return ReturnResult();
}

void SceneGame::Draw( float elapsedTime )
{
	// Draw BackGround.
#if DEBUG_MODE

	Donya::Sprite::DrawRect
	(
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		Common::ScreenWidthF(),
		Common::ScreenHeightF(),
		Donya::Sprite::Color::TEAL, 1.0f
	);

	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		"Game",
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		32.0f, 32.0f,
		32.0f, 32.0f
	);

#endif // DEBUG_MODE

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

	pImpl->particleManager.Draw( matView, matProj, lightDir, cameraPos );

	for ( const auto &it : pImpl->reflectedEntities )
	{
		it.Draw( matView, matProj, lightDir, cameraPos );
	}

	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		pImpl->currentTime.ToStr(),
		32.0f, 64.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);
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

	AABB playerBox = pImpl->player.GetHitBox();

	// Missiles vs Player.
	{
		AABB other{};
		auto &reflectableAttacks = pImpl->boss.FetchReflectableMissiles();
		for ( const auto &it : reflectableAttacks )
		{
			other = it.GetHitBox();
			if ( AABB::IsHitAABB( other, playerBox ) )
			{
				it.HitToOther();

				HitToPlayer( /* canReflection = */ true );
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
	// Obstacles vs Player.
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
	if ( requestPause && !Fader::Get().IsExist() )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result pause{};
		pause.AddRequest( Scene::Request::ADD_SCENE );
		pause.sceneType = Scene::Type::Pause;
		return pause;
	}
	// else

#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_RETURN ) )
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