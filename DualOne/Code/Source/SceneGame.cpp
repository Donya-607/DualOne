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

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"

struct SceneGame::Impl
{
public:
	size_t sprFont;
public:
	Impl() : sprFont( NULL )
	{}
	~Impl()
	{

	}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		//archive
		//(
		//	CEREAL_NVP()
		//);

		if ( 1 <= version )
		{
			//archive
			//(
			//	CEREAL_NVP()
			//);
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
				if ( ImGui::TreeNode( "Xbox Input" ) )
				{
					ImGui::Text( "Input Check." );

					static Donya::XInput controller{ Donya::XInput::PadNumber::PAD_1 };
					controller.Update();

					using Button = Donya::XInput::Button;
					std::vector<std::string> inputs
					{
						"UP : " + std::to_string( controller.Press( Button::UP ) ),
						"DOWN : " + std::to_string( controller.Press( Button::DOWN ) ),
						"LEFT : " + std::to_string( controller.Press( Button::LEFT ) ),
						"RIGHT : " + std::to_string( controller.Press( Button::RIGHT ) ),
						"START : " + std::to_string( controller.Press( Button::START ) ),
						"SELECT : " + std::to_string( controller.Press( Button::SELECT ) ),
						"PRESS_L : " + std::to_string( controller.Press( Button::PRESS_L ) ),
						"PRESS_R : " + std::to_string( controller.Press( Button::PRESS_R ) ),
						"LB : " + std::to_string( controller.Press( Button::LB ) ),
						"RB : " + std::to_string( controller.Press( Button::RB ) ),
						"A : " + std::to_string( controller.Press( Button::A ) ),
						"B : " + std::to_string( controller.Press( Button::B ) ),
						"X : " + std::to_string( controller.Press( Button::X ) ),
						"Y : " + std::to_string( controller.Press( Button::Y ) ),
						"LT : " + std::to_string( controller.Press( Button::LT ) ),
						"RT : " + std::to_string( controller.Press( Button::RT ) ),
						"Stick L X : " + std::to_string( controller.LeftStick().x ),
						"Stick L Y : " + std::to_string( controller.LeftStick().y ),
						"Stick R X : " + std::to_string( controller.RightStick().x ),
						"Stick R Y : " + std::to_string( controller.RightStick().y ),
					};

					auto ShowString = []( const std::string &str )
					{
						ImGui::Text( str.c_str() );
					};
					for ( size_t i = 0; i < inputs.size(); ++i )
					{
						ShowString( inputs[i] );
					}

					ImGui::TreePop();
				}

				ImGui::Text( "" );

				if ( ImGui::TreeNode( u8"ファイル（タイマー）" ) )
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
}

void SceneGame::Uninit()
{
	pImpl->SaveParameter();

	Donya::ScreenShake::StopX();
	Donya::ScreenShake::StopY();

	Donya::Sound::Stop( Music::BGM_Game );
}

Scene::Result SceneGame::Update( float elapsedTime )
{
#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	return ReturnResult();
}

void SceneGame::Draw( float elapsedTime )
{
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