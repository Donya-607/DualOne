#include "ScenePause.h"

#include <algorithm>

#include "Donya/Constant.h"
#include "Donya/Keyboard.h"
#include "Donya/Sprite.h"
#include "Donya/Sound.h"
#include "Donya/Vector.h"

#include "Common.h"
#include "FilePath.h"
#include "Music.h"

#undef max
#undef min

ScenePause::ScenePause() :
	choice( Choice::Resume ),
	sprUI(),
	controller( Donya::XInput::PadNumber::PAD_1 )
{

}
ScenePause::~ScenePause()
{

}

void ScenePause::Init()
{
	sprUI.LoadSheet( GetSpritePath( SpriteAttribute::UI ), 256U );
}

void ScenePause::Uninit()
{

}

Scene::Result ScenePause::Update( float elapsedTime )
{
	controller.Update();

	UpdateChooseItem();

	return ReturnResult();
}

void ScenePause::Draw( float elapsedTime )
{
	
}

void ScenePause::UpdateChooseItem()
{
	bool left{}, right{};
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

Scene::Result ScenePause::ReturnResult()
{
	if ( Donya::Keyboard::Trigger( 'P' ) || controller.Trigger( Donya::Gamepad::Button::START ) )
	{
		Donya::Sound::Play( Music::ItemDecision );

		Scene::Result change{};
		change.AddRequest( Scene::Request::REMOVE_ME );
		return change;
	}

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
