#include "ScenePause.h"

#include <algorithm>

#include "Donya/Constant.h"
#include "Donya/Keyboard.h"
#include "Donya/Sprite.h"
#include "Donya/Sound.h"
#include "Donya/Vector.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"

#undef max
#undef min

ScenePause::ScenePause() :
	choice( Choice::Resume ),
	sprFont( NULL ),
	controller( Donya::XInput::PadNumber::PAD_1 )
{

}
ScenePause::~ScenePause()
{

}

void ScenePause::Init()
{
	sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::TestFont ), 1024U );
}

void ScenePause::Uninit()
{

}

Scene::Result ScenePause::Update( float elapsedTime )
{
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

	auto GetString = []( Choice choice )->std::string
	{
		switch ( choice )
		{
		case ScenePause::BackToTitle:	return "  Back to Title >";	break;
		case ScenePause::Resume:		return "< Resume        >";	break;
		case ScenePause::ReTry:			return "< Retry";			break;
		default: break;
		}
		return "";
	};
	Donya::Sprite::DrawString
	(
		sprFont,
		GetString( choice ).c_str(),
		512.0f, 128.0f,
		32.0f, 32.0f,
		32.0f, 32.0f
	);
}

void ScenePause::UpdateChooseItem()
{
	bool left{}, right{};
	left  = controller.Trigger( Donya::Gamepad::Button::LEFT  ) || ( Donya::Keyboard::Trigger( VK_LEFT  ) ? true : false );
	right = controller.Trigger( Donya::Gamepad::Button::RIGHT ) || ( Donya::Keyboard::Trigger( VK_RIGHT ) ? true : false );
	if ( !left )
	{
		left  = controller.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
	}
	if ( !right )
	{
		right = controller.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
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
	(
		Donya::Keyboard::Trigger( 'Z' ) ||
		controller.Trigger( Donya::Gamepad::A )
	)
	? true
	: false;
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
	bool chooseResume = ( !ShouldUseFade( choice ) && IsDecisionTriggered() );
	if ( Donya::Keyboard::Trigger( 'P' ) || controller.Trigger( Donya::Gamepad::Button::START ) || chooseResume )
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
		}

		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
