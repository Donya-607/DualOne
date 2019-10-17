#include "SceneClear.h"

#include <algorithm>
#include <string>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

#include "Donya/GamepadXInput.h"
#include "Donya/Keyboard.h"
#include "Donya/Random.h"
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
#include "StorageForScene.h"
#include "Timer.h"

#undef min
#undef max

class SceneClear::Impl
{
public:
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		ReTry,
	};
public:
	size_t			sprFont;
	Choice			choice;
	Timer			clearTime;
	Timer			bestTime;
	std::string		timeString;		// Store the "clearTime" contents. looks like "12:34:56"(min-sec-ms).
	std::string		bestTimeString;	// Store the "bestTime" contents. looks like "12:34:56"(min-sec-ms).
	Donya::XInput	controller;
	bool			wasNewRecord;	// True if the "clearTime" faster than "bestTime".
public:
	Impl() : sprFont( NULL ),
		choice( Nil ),
		clearTime(), bestTime(),
		timeString(), bestTimeString(),
		controller( Donya::XInput::PadNumber::PAD_1 ),
		wasNewRecord( false )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		archive( CEREAL_NVP( bestTime ) );

		if ( 1 <= version )
		{
			/*
			archive( CEREAL_NVP() );
			*/
		}
	}
	static constexpr const char *SERIAL_ID = "Result";
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
			if ( ImGui::TreeNode( u8"リザルト" ) )
			{
				static int exCurrent{}, exSecond{}, exMinute{};
				ImGui::SliderInt( u8"ベストタイム・分",		&exMinute,  0, 99 );
				ImGui::SliderInt( u8"ベストタイム・秒",		&exSecond,  0, 59 );
				ImGui::SliderInt( u8"ベストタイム・ミリ秒",	&exCurrent, 0, 59 );
				if ( ImGui::Button( u8"ベストタイム上書き" ) )
				{
					bestTime.Set( exMinute, exSecond, exCurrent );
					bestTimeString = bestTime.ToStr();
				}
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

			ImGui::End();
		}
	}

#endif // USE_IMGUI
};

SceneClear::SceneClear() : pImpl( std::make_unique<SceneClear::Impl>() )
{

}
SceneClear::~SceneClear()
{
	pImpl.reset( nullptr );
}

void SceneClear::Init()
{
	Donya::Sound::Play( Music::BGM_Clear );

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::Font ), 1024U );

	pImpl->bestTime.Set( 99, 59, 59 );
	pImpl->LoadParameter(); // Load best-time.

	pImpl->clearTime  = StorageForScene::Get().GetTimer();
	pImpl->timeString = pImpl->clearTime.ToStr();

	// If faster than bestTime.
	if ( pImpl->clearTime < pImpl->bestTime )
	{
		pImpl->bestTime = pImpl->clearTime;
		pImpl->bestTimeString = pImpl->timeString;

		pImpl->SaveParameter();

		pImpl->wasNewRecord = true;
	}
	else
	{
		pImpl->bestTimeString = pImpl->bestTime.ToStr();
	}
}

void SceneClear::Uninit()
{
	Donya::Sound::Stop( Music::BGM_Clear );
}

Scene::Result SceneClear::Update( float elapsedTime )
{
#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->controller.Update();

	UpdateChooseItem();

	bool canDecision = ( !Fader::Get().IsExist() && pImpl->choice != Impl::Choice::Nil );
	if ( canDecision && IsDecisionTriggered() )
	{
		Donya::Sound::Play( Music::ItemDecision );

		StartFade();
	}

	return ReturnResult();
}

void SceneClear::Draw( float elapsedTime )
{
	Donya::Sprite::SetDrawDepth( 0.0f );

	// Darken.
	Donya::Sprite::DrawRect
	(
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		Common::ScreenWidthF(),
		Common::ScreenHeightF(),
		Donya::Sprite::Color::BLACK, 0.6f
	);

	Donya::Sprite::DrawStringExt
	(
		pImpl->sprFont,
		"Clear",
		Common::HalfScreenWidthF(),
		64.0f,
		32.0f, 32.0f,
		32.0f, 32.0f,
		2.0f, 2.0f
	);

	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		pImpl->timeString,
		32.0f, 64.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);
	std::string bestTimeDesc{};
	{
		bestTimeDesc = "BestTime : ";

		if ( pImpl->wasNewRecord )
		{
			bestTimeDesc += "New Record!";
		}
	}
	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		bestTimeDesc,
		32.0f, 128.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);
	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		pImpl->bestTimeString,
		32.0f, 160.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);

	auto GetString = []( Impl::Choice choice )->std::string
	{
		switch ( choice )
		{
		case Impl::Choice::Nil:			return "<Choose by arrow>";	break;
		case Impl::Choice::BackToTitle:	return "  Back to Title >";	break;
		case Impl::Choice::ReTry:		return "< Retry";			break;
		default: break;
		}
		return "";
	};
	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		GetString( pImpl->choice ).c_str(),
		Common::HalfScreenWidthF(),
		128.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);
}

void SceneClear::UpdateChooseItem()
{
	bool left{}, right{};
	
	if ( IsConrollerConnected() )
	{
		left  = pImpl->controller.Trigger( Donya::Gamepad::Button::LEFT  );
		right = pImpl->controller.Trigger( Donya::Gamepad::Button::RIGHT );
		if ( !left )
		{
			left  = pImpl->controller.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
		}
		if ( !right )
		{
			right = pImpl->controller.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
		}
	}
	else
	{
		left  = Donya::Keyboard::Trigger( VK_LEFT  ) ? true : false;
		right = Donya::Keyboard::Trigger( VK_RIGHT ) ? true : false;
	}

	int index = scast<int>( pImpl->choice );
	int oldIndex = index;

	if ( pImpl->choice == Impl::Choice::Nil )
	{
		if ( left  ) { index = scast<int>( Impl::Choice::BackToTitle	); }
		if ( right ) { index = scast<int>( Impl::Choice::ReTry			); }
	}
	else
	{
		if ( left  ) { index--; }
		if ( right ) { index++; }

		index = std::max( 0, std::min( scast<int>( Impl::Choice::ReTry ), index ) );
	}

	if ( index != oldIndex )
	{
		Donya::Sound::Play( Music::ItemChoose );
	}

	pImpl->choice = scast<Impl::Choice>( index );
}

bool SceneClear::IsConrollerConnected() const
{
	return pImpl->controller.IsConnected();
}

bool SceneClear::IsDecisionTriggered() const
{
	return
	( IsConrollerConnected() )
	? pImpl->controller.Trigger( Donya::Gamepad::A )
	: ( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false;
}

void SceneClear::StartFade()
{
	Fader::Configuration config{};
	config.type			= Fader::Type::Gradually;
	config.closeFrame	= 20;
	config.parameter	= scast<unsigned int>( Donya::Sprite::Color::BLACK );
	Fader::Get().StartFadeOut( config );
}

Scene::Result SceneClear::ReturnResult()
{
	if ( Fader::Get().IsClosed() )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ALL );

		if ( pImpl->choice == Impl::Choice::BackToTitle )
		{
			change.sceneType = Scene::Type::Title;
		}
		else
		if ( pImpl->choice == Impl::Choice::ReTry )
		{
			change.sceneType = Scene::Type::Game;
			StorageForScene::Get().StoreRetryFlag( true );
		}
		else
		{
			_ASSERT_EXPR( 0, L"Error : The chosen item is invalid !" );
		}

		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
