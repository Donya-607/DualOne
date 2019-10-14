#include "SceneOver.h"

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
	size_t			sprFont;
	Choice			choice;
	Donya::XInput	controller;
public:
	Impl() : sprFont( NULL ),
		choice( Nil ),
		controller( Donya::XInput::PadNumber::PAD_1 )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		// archive( CEREAL_NVP() );
	}
public:
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

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::TestFont ), 1024U );
}

void SceneOver::Uninit()
{
	Donya::Sound::Stop( Music::BGM_Over );
}

Scene::Result SceneOver::Update( float elapsedTime )
{
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

	Donya::Sprite::DrawStringExt
	(
		pImpl->sprFont,
		"Over",
		Common::HalfScreenWidthF(),
		64.0f,
		32.0f, 32.0f,
		32.0f, 32.0f,
		2.0f, 2.0f
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
