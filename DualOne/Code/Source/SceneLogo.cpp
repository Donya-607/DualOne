#include "SceneLogo.h"

#include "Donya/Constant.h"
#include "Donya/Keyboard.h"
#include "Donya/Sprite.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"

SceneLogo::SceneLogo() :
	status( State::FADE_IN ),
	sprRightsLogo( NULL ),
	showIndex( 0 ), showCount( 1 ), timer( 0 ),
	alpha( 0.0f ), scale( 1.0f )
{
	sprRightsLogo = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::FMODLogoBlack ), 2U );
}
SceneLogo::~SceneLogo()
{

}

void SceneLogo::Init()
{

}

void SceneLogo::Uninit()
{

}

Scene::Result SceneLogo::Update( float elapsedTime )
{
	constexpr int FADE_IN_FRAME		= 20;
	constexpr int WAIT_FRAME		= 45;
	constexpr int FADE_OUT_FRAME	= 20;

	constexpr float FADE_IN_SPEED	= 1.0f / scast<float>( FADE_IN_FRAME  );
	constexpr float FADE_OUT_SPEED	= 1.0f / scast<float>( FADE_OUT_FRAME );

	timer++;

	if ( Donya::Keyboard::Trigger( VK_RETURN ) )
	{
		if ( status != State::FADE_OUT && status != State::END )
		{
			status = State::FADE_OUT;
			timer = 0;
			alpha = 1.0f;
		}
	}

	switch ( status )
	{
	case SceneLogo::State::FADE_IN:

		alpha += FADE_IN_SPEED;

		if ( FADE_IN_FRAME <= timer )
		{
			timer  = 0;
			alpha  = 1.0f;
			status = State::WAIT;
		}
		break;
	case SceneLogo::State::WAIT:
		if ( WAIT_FRAME <= timer )
		{
			timer = 0;
			status = State::FADE_OUT;
		}
		break;
	case SceneLogo::State::FADE_OUT:

		alpha -= FADE_OUT_SPEED;

		if ( FADE_OUT_FRAME <= timer )
		{
			timer = 0;
			alpha = 0.0f;
			showIndex++;
			
			if ( showCount <= showIndex )
			{
				Fader::Configuration config{};
				config.type = Fader::Type::Scroll;
				config.closeFrame	= 20;
				config.parameter	= Fader::Direction::RIGHT;
				Fader::Get().StartFadeOut( config );

				status = State::END;
			}
			else
			{
				status = State::FADE_IN;
			}
		}
		break;
	default: break;
	}

	return ReturnResult();
}

void SceneLogo::Draw( float elapsedTime )
{
	Donya::Sprite::DrawRect
	(
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		Common::ScreenWidthF(),
		Common::ScreenHeightF(),
		Donya::Sprite::Color::GRAY, 1.0f
	);

	Donya::Sprite::DrawExt
	(
		sprRightsLogo,
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		scale, scale,
		0.0f, alpha
	);
}

Scene::Result SceneLogo::ReturnResult()
{
	if ( Fader::Get().IsClosed() )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Title;
		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
