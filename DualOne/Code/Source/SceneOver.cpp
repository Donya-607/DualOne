#include "SceneOver.h"

#include <algorithm>
#include <string>

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
#include "Sentence.h"
#include "StorageForScene.h"
#include "Timer.h"

#undef min
#undef max

class SceneOver::Impl
{
public:
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		ReTry,
	};
public:
	Choice			choice;

	Donya::Vector2	uiOverPos;
	Donya::Vector2	uiBackPos;
	Donya::Vector2	uiRetryPos;
	Donya::Vector2	uiArrowPosL;
	Donya::Vector2	uiArrowPosR;

	Donya::XInput	controller;
public:
	Impl() :
		choice( Nil ),
		uiOverPos(), uiBackPos(), uiRetryPos(),
		controller( Donya::XInput::PadNumber::PAD_1 )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( uiOverPos ),
			CEREAL_NVP( uiBackPos ),
			CEREAL_NVP( uiRetryPos ),
			CEREAL_NVP( uiArrowPosL ),
			CEREAL_NVP( uiArrowPosR )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP() );
		}
	}
	static constexpr const char *SERIAL_ID = "Over";
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
			if ( ImGui::TreeNode( u8"ゲームオーバー画面" ) )
			{
				auto ShowVec2 = []( const std::string &caption, Donya::Vector2 *pV )
				{
					if ( !pV ) { return; }
					// else
					ImGui::SliderFloat2( caption.c_str(), &pV->x, 0.0f, 1920.0f );
				};

				ShowVec2( u8"ゲームオーバー・描画位置",	&uiOverPos		);
				ShowVec2( u8"リトライ・描画位置",			&uiRetryPos		);
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
};

SceneOver::SceneOver() : pImpl( std::make_unique<SceneOver::Impl>() )
{

}
SceneOver::~SceneOver()
{
	pImpl.reset( nullptr );
}

void SceneOver::Init()
{
	Donya::Sound::Play( Music::BGM_Over );

	pImpl->LoadParameter();
}

void SceneOver::Uninit()
{
	Donya::Sound::Stop( Music::BGM_Over );
}

Scene::Result SceneOver::Update( float elapsedTime )
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

void SceneOver::Draw( float elapsedTime )
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
	switch ( pImpl->choice )
	{
	case Impl::BackToTitle:
		GET.Draw( Sentence::Kind::BackToTitle, pImpl->uiBackPos );
		break;
	case Impl::ReTry:
		GET.Draw( Sentence::Kind::Retry, pImpl->uiRetryPos );
		break;
	default: break;
	}
	GET.Draw( Sentence::Kind::GAME_OVER, pImpl->uiOverPos );
	if ( pImpl->choice != Impl::Choice::BackToTitle	) { GET.Draw( Sentence::Kind::LeftArrow,  pImpl->uiArrowPosL ); }
	if ( pImpl->choice != Impl::Choice::ReTry		) { GET.Draw( Sentence::Kind::RightArrow, pImpl->uiArrowPosR ); }
}

void SceneOver::UpdateChooseItem()
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

bool SceneOver::IsConrollerConnected() const
{
	return pImpl->controller.IsConnected();
}

bool SceneOver::IsDecisionTriggered() const
{
	return
	( IsConrollerConnected() )
	? pImpl->controller.Trigger( Donya::Gamepad::A )
	: ( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false;
}

void SceneOver::StartFade()
{
	Fader::Configuration config{};
	config.type			= Fader::Type::Gradually;
	config.closeFrame	= 20;
	config.parameter	= scast<unsigned int>( Donya::Sprite::Color::BLACK );
	Fader::Get().StartFadeOut( config );
}

Scene::Result SceneOver::ReturnResult()
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
