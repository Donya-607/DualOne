#include "SceneTitle.h"

#include <algorithm>
#include <string>
#include <vector>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "Donya/GamepadXInput.h"
#include "Donya/Keyboard.h"
#include "Donya/Serializer.h"
#include "Donya/Sound.h"
#include "Donya/Sprite.h"
#include "Donya/Useful.h"
#include "Donya/Vector.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"

#undef min
#undef max

class SceneTitle::Impl
{
public:
	size_t			sprFont;
	Donya::XInput	controller;
	bool			wasAddedGameScene; // Append the game-scene below me(title).
public:
	Impl() : sprFont( NULL ),
		controller( Donya::Gamepad::PadNumber::PAD_1 ),
		wasAddedGameScene( false )
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

	pImpl->sprFont = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::TestFont ), 1024U );
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

	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		"Title",
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		32.0f, 32.0f,
		32.0f, 32.0f
	);
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
