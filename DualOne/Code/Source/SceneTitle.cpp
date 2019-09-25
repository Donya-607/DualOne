#include "SceneTitle.h"

#include <algorithm>
#include <string>
#include <vector>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "Donya/Blend.h"
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

#undef min
#undef max

class SceneTitle::Impl
{
public:
	size_t	sprFont;
public:
	Impl() : sprFont( NULL )
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
	return ReturnResult();
}

void SceneTitle::Draw( float elapsedTime )
{
	Donya::Sprite::DrawString
	(
		pImpl->sprFont,
		"Pause",
		Common::HalfScreenWidthF(),
		Common::HalfScreenHeightF(),
		32.0f, 32.0f,
		32.0f, 32.0f
	);
}

Scene::Result SceneTitle::ReturnResult()
{
#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_BACK ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Logo;
		return change;
	}
	// else
	if ( Donya::Keyboard::Trigger( VK_RETURN ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Game;
		return change;
	}
	// else
#endif // DEBUG_MODE

	//if ( pImpl->changeScene )
	//{
	//	Scene::Result change{};
	//	change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
	//	change.sceneType = Scene::Type::Game;
	//	return change;
	//}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
