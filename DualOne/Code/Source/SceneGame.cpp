#include "SceneGame.h"

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
#include "Music.h"
#include "Player.h"

struct SceneGame::Impl
{
public:
	size_t sprFont;
	Camera camera;
	Player player;

	Donya::Vector3 lightDirection;
public:
	Impl() : sprFont( NULL ),
		camera(),
		player(),
		lightDirection( 0.0f, 0.0f, 1.0f )
	{}
	~Impl()
	{}
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
			/*
			archive
			(
				CEREAL_NVP()
			);
			*/
		}
	}
	static constexpr const char *SERIAL_ID = "Game";
public:
	void LoadParameter( bool isBinary = true )
	{
		Serializer::Extension ext =	( isBinary )
									? Serializer::Extension::BINARY
									: Serializer::Extension::JSON;
		std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

		Serializer seria;
		seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
	}
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
public:
#if USE_IMGUI

	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( "Game" ) )
			{
				ImGui::SliderFloat3( u8"ライトの方向", &lightDirection.x, -4.0f, 4.0f );
				ImGui::Text( "" );

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

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::TestFont ), 1024U );

	constexpr float FOV = ToRadian( 30.0f );
	pImpl->camera.Init( Common::ScreenWidthF(), Common::ScreenHeightF(), FOV );
	pImpl->camera.SetFocusCoordinate( { 0.0f, 0.0f, 1.0f } );

	pImpl->player.Init();
}

void SceneGame::Uninit()
{
	pImpl->player.Uninit();

	Donya::ScreenShake::StopX();
	Donya::ScreenShake::StopY();

	Donya::Sound::Stop( Music::BGM_Game );
}

Scene::Result SceneGame::Update( float elapsedTime )
{
#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->player.Update();

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
		ctrl.moveVelocity		= movement;
		ctrl.rotation			= rotation;
		ctrl.slerpPercent		= 1.0f;
		ctrl.moveAtLocalSpace	= true;

		return ctrl;
	};
	cameraController = MakeControlStructWithMouse();
	if ( Donya::Keyboard::Trigger( 'R' ) )
	{
		pImpl->camera.SetPosition( { 0.0f, 0.0f, 0.0f } );
	}
#else

#endif // DEBUG_MODE

	pImpl->camera.Update( cameraController );

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
		return Donya::Vector4 { vec3.x, vec3.y, vec3.z, fourthValue };
	};

	XMFLOAT4X4 matView = Float4x4( pImpl->camera.CalcViewMatrix()		);
	XMFLOAT4X4 matProj = Float4x4( pImpl->camera.GetProjectionMatrix()	);
	Donya::Vector4 lightDir  = ToFloat4( pImpl->lightDirection,  0.0f	);
	Donya::Vector4 cameraPos = ToFloat4( pImpl->camera.GetPos(), 1.0f	);

	pImpl->player.Draw( matView, matProj, lightDir, cameraPos );
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

	if ( Donya::Keyboard::Trigger( 'P' ) )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result pause{};
		pause.AddRequest( Scene::Request::ADD_SCENE );
		pause.sceneType = Scene::Type::Pause;
		return pause;
	}

#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_RETURN ) )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Title;
		return change;
	}
	// else
#endif // DEBUG_MODE

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}