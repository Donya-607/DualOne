#include "SceneTitle.h"

#include <string>

#include "Donya/GamepadXInput.h"
#include "Donya/Keyboard.h"
#include "Donya/Serializer.h"
#include "Donya/Sound.h"
#include "Donya/Sprite.h"
#include "Donya/Useful.h"
#include "Donya/UseImgui.h"
#include "Donya/Vector.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"
#include "UI.h"

#undef min
#undef max

class SceneTitle::Impl
{
public:
	UIObject		uiTitleLogo;

	Donya::Vector2	uiButtonPos;		// Sentence position.
	Donya::Vector2	uiHoldPos;			// Sentence position.
	Donya::Vector2	uiReleasePos;		// Sentence position.

	Donya::XInput	controller;
	bool			wasAddedGameScene;	// Append the game-scene below me(title).
	bool			isHolding;
public:
	Impl() :
		uiTitleLogo(), uiButtonPos(), uiHoldPos(), uiReleasePos(),
		controller( Donya::Gamepad::PadNumber::PAD_1 ),
		wasAddedGameScene( false ), isHolding( false )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( uiTitleLogo ),
			CEREAL_NVP( uiButtonPos ),
			CEREAL_NVP( uiHoldPos ),
			CEREAL_NVP( uiReleasePos )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP() );
		}
	}
	static constexpr const char *SERIAL_ID = "Title";
public:
	void LoadParameter( bool isBinary )
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
			if ( ImGui::TreeNode( u8"タイトル画面" ) )
			{
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

			ImGui::End();
		}
	}

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( SceneTitle::Impl, 0 )

SceneTitle::SceneTitle() : pImpl( std::make_unique<SceneTitle::Impl>() )
{
	
}
SceneTitle::~SceneTitle()
{
	pImpl.reset( nullptr );
}

void SceneTitle::Init()
{
	Donya::Sound::Play( Music::BGM_Title );

	pImpl->uiTitleLogo.LoadSprite( GetSpritePath( SpriteAttribute::TitleLogo ), 2U );
}

void SceneTitle::Uninit()
{
	Donya::Sound::Stop( Music::BGM_Title );
}

Scene::Result SceneTitle::Update( float elapsedTime )
{
	pImpl->controller.Update();

	/*
	if ( IsDecisionTriggered() && !Fader::Get().IsExist() )
	{
		Donya::Sound::Play( Music::ItemDecision );

		StartFade();
	}
	*/

	return ReturnResult();
}

void SceneTitle::Draw( float elapsedTime )
{
	Donya::Sprite::SetDrawDepth( 0.0f );

	
}

bool SceneTitle::IsDecisionTriggered() const
{
	return
	( pImpl->controller.IsConnected() )
	? pImpl->controller.Trigger( Donya::Gamepad::A )
	: ( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false;
}
bool SceneTitle::IsDecisionReleased() const
{
	return
	( pImpl->controller.IsConnected() )
	? pImpl->controller.Release( Donya::Gamepad::A )
	: ( Donya::Keyboard::Release( 'Z' ) ) ? true : false;
}

void SceneTitle::StartFade()
{
	Fader::Configuration config{};
	config.type			= Fader::Type::Gradually;
	config.closeFrame	= 20;
	config.parameter	= scast<unsigned int>( Donya::Sprite::Color::BLACK );
	Fader::Get().StartFadeOut( config );
}

Scene::Result SceneTitle::ReturnResult()
{
#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_BACK ) && Donya::Keyboard::Press( VK_RSHIFT ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ALL );
		change.sceneType = Scene::Type::Logo;
		return change;
	}
	// else
#endif // DEBUG_MODE

	if ( !pImpl->wasAddedGameScene )
	{
		pImpl->wasAddedGameScene = true;

		Scene::Result append{};
		append.AddRequest( Scene::Request::UPDATE_NEXT, Scene::Request::APPEND_SCENE );
		append.sceneType = Scene::Type::Game;
		return append;
	}

	// if ( Fader::Get().IsClosed() )
	if ( IsDecisionReleased() )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result change{};
		change.AddRequest( Scene::Request::UPDATE_NEXT, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Null;
		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::UPDATE_NEXT, Scene::Type::Null };
	return noop;
}
