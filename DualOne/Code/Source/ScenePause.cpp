#include "ScenePause.h"

#include <algorithm>

#include "Donya/Constant.h"
#include "Donya/Keyboard.h"
#include "Donya/Sprite.h"
#include "Donya/Sound.h"
#include "Donya/Vector.h"
#include "Donya/Useful.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"
#include "Sentence.h"
#include "StorageForScene.h"

#undef max
#undef min

ScenePause::ScenePause() :
	choice( Choice::Resume ),
	uiPausePos(), uiRetryPos(), uiResumePos(), uiBackPos(),
	controller( Donya::XInput::PadNumber::PAD_1 )
{

}
ScenePause::~ScenePause() = default;

void ScenePause::Init()
{
	LoadParameter();

	// Suppress continiously trigger pause-button.
	controller.Update();
}

void ScenePause::Uninit()
{

}

Scene::Result ScenePause::Update( float elapsedTime )
{
#if USE_IMGUI

	UseImGui();

#endif // USE_IMGUI

	controller.Update();

	UpdateChooseItem();

	if ( !Fader::Get().IsExist() && IsDecisionTriggered() )
	{
		if ( ShouldUseFade( choice ) )
		{
			Donya::Sound::Play( Music::ItemDecision );
			
			StartFade();
		}
	}

	return ReturnResult();
}

void ScenePause::Draw( float elapsedTime )
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

	const auto &GET = Sentence::Get();
	switch ( choice )
	{
	case ScenePause::BackToTitle:
		GET.Draw( Sentence::Kind::BackToTitle, uiBackPos );
		break;
	case ScenePause::Resume:
		GET.Draw( Sentence::Kind::BackToGame, uiResumePos );
		break;
	case ScenePause::ReTry:
		GET.Draw( Sentence::Kind::Retry, uiRetryPos );
		break;
	default: break;
	}
	GET.Draw( Sentence::Kind::Pause,		uiPausePos  );
	if( choice != ScenePause::Choice::BackToTitle	) { GET.Draw( Sentence::Kind::LeftArrow,	uiArrowPosL ); }
	if( choice != ScenePause::Choice::ReTry			) { GET.Draw( Sentence::Kind::RightArrow,	uiArrowPosR ); }
}

void ScenePause::UpdateChooseItem()
{
	bool left{}, right{};
	if ( controller.IsConnected() )
	{
		left  = controller.Trigger( Donya::Gamepad::Button::LEFT );
		right = controller.Trigger( Donya::Gamepad::Button::RIGHT );
		if ( !left )
		{
			left  = controller.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
		}
		if ( !right )
		{
			right = controller.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
		}
	}
	else
	{
		left  = Donya::Keyboard::Trigger( VK_LEFT  ) ? true : false;
		right = Donya::Keyboard::Trigger( VK_RIGHT ) ? true : false;
	}

	int index = scast<int>( choice );
	int oldIndex = index;

	if ( left ) { index--; }
	if ( right ) { index++; }

	index = std::max( 0, std::min( scast<int>( Choice::ReTry ), index ) );

	if ( index != oldIndex )
	{
		Donya::Sound::Play( Music::ItemChoose );
	}

	choice = scast<Choice>( index );
}

bool ScenePause::IsDecisionTriggered() const
{
	return
	( controller.IsConnected() )
	? controller.Trigger( Donya::Gamepad::A )
	: ( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false;
}

bool ScenePause::ShouldUseFade( Choice choice ) const
{
	switch ( choice )
	{
	case ScenePause::BackToTitle:	return true;
	case ScenePause::ReTry:			return true;
	default: break;
	}

	return false;
}

void ScenePause::StartFade()
{
	Fader::Configuration config{};
	config.type			= Fader::Type::Gradually;
	config.closeFrame	= 20;
	config.parameter	= scast<unsigned int>( Donya::Sprite::Color::BLACK );
	Fader::Get().StartFadeOut( config );
}

Scene::Result ScenePause::ReturnResult()
{
	// Resume.
	bool chooseResume	= ( !ShouldUseFade( choice ) && IsDecisionTriggered() );
	bool pushResume		= ( Donya::Keyboard::Trigger( 'P' ) || controller.Trigger( Donya::Gamepad::Button::START ) || controller.Trigger( Donya::Gamepad::Button::SELECT ) || chooseResume );
	if ( pushResume && !Fader::Get().IsExist() )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result change{};
		change.AddRequest( Scene::Request::REMOVE_ME );
		return change;
	}
	// else

	if ( Fader::Get().IsClosed() )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ALL );

		if ( choice == Choice::BackToTitle )
		{
			change.sceneType = Scene::Type::Title;
		}
		else
		if ( choice == Choice::ReTry )
		{
			change.sceneType = Scene::Type::Game;
			StorageForScene::Get().StoreRetryFlag( true );
		}
		else
		{
			_ASSERT_EXPR( 0, L"Error : You chosen scene is invalid !" );
			change.sceneType = Scene::Type::Logo; // Fail-safe
		}

		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}

void ScenePause::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void ScenePause::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void ScenePause::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"ポーズ画面" ) )
		{
			auto ShowVec2 = []( const std::string &caption, Donya::Vector2 *pV )
			{
				if ( !pV ) { return; }
				// else
				ImGui::SliderFloat2( caption.c_str(), &pV->x, 0.0f, 1920.0f );
			};

			ShowVec2( u8"ポーズ・描画位置",			&uiPausePos		);
			ShowVec2( u8"リトライ・描画位置",			&uiRetryPos		);
			ShowVec2( u8"再開・描画位置",				&uiResumePos	);
			ShowVec2( u8"タイトルへ戻る・描画位置",	&uiBackPos		);
			ShowVec2( u8"左矢印・描画位置",			&uiArrowPosL	);
			ShowVec2( u8"右矢印・描画位置",			&uiArrowPosR	);

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
