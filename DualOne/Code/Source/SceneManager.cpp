#include "SceneManager.h"

#include <algorithm>

#include "Donya/Resource.h"
#include "Donya/Sprite.h"	// For change the sprites depth.

#include "Common.h"
#include "Fader.h"
#include "Scene.h"
#include "SceneClear.h"
#include "SceneGame.h"
#include "SceneLogo.h"
#include "ScenePause.h"
#include "SceneOver.h"
#include "SceneTitle.h"

#undef max
#undef min

SceneMng::SceneMng() : pScenes()
{

}
SceneMng::~SceneMng()
{
	pScenes.clear();
}

void SceneMng::Init( Scene::Type initScene )
{
	PushScene( initScene, /* isFront = */ true );

	Fader::Get().Init();
}

void SceneMng::Uninit()
{
	for ( auto &it : pScenes )
	{
		it->Uninit();
	}

	pScenes.clear();

	Fader::Get().Init();

	Donya::Resource::ReleaseAllCachedResources();
}

void SceneMng::Update( float elapsedTime )
{
	if ( pScenes.empty() )
	{
		static BOOL NO_EXPECT_ERROR = TRUE;
		PushScene( Scene::Type::Title, true );
	}

	Scene::Result message{};

	int updateCount = 1;
	for ( int i = 0; i < updateCount; ++i )
	{
		auto &itr = ( *std::next( pScenes.begin(), i ) );
		message = itr->Update( elapsedTime );

		ProcessMessage( message, updateCount, i );
	}

	Fader::Get().Update();
}

void SceneMng::Draw( float elapsedTime )
{
	Donya::Sprite::SetDrawDepth( 1.0f );

	const auto &end = pScenes.crend();
	for ( auto it   = pScenes.crbegin(); it != end; ++it )
	{
		( *it )->Draw( elapsedTime );
	}

	Donya::Sprite::SetDrawDepth( 0.0f );

	Fader::Get().Draw();
}

bool SceneMng::WillEmptyIfApplied( Scene::Result message ) const
{
	if ( message.request == Scene::Request::NONE ) { return false; }
	// else

	bool willEmpty = false;

	if ( message.HasRequest( Scene::Request::REMOVE_ALL ) )
	{
		willEmpty = true;
	}
	if ( message.HasRequest( Scene::Request::REMOVE_ME ) && pScenes.size() == 1 )
	{
		willEmpty = true;
	}

	if ( willEmpty && message.sceneType == Scene::Type::Null )
	{
		return true;
	}

	return false;
}
bool SceneMng::ValidateMessage( Scene::Result message ) const
{
	if ( message.request == Scene::Request::NONE ) { return true; }
	// else

	if ( WillEmptyIfApplied( message ) )
	{
		return false;
	}

	return true;
}
Scene::Result SceneMng::ApplyFailSafe( Scene::Result wrongMessage ) const
{
	if ( WillEmptyIfApplied( wrongMessage ) )
	{
		wrongMessage.sceneType = Scene::Type::Logo;
	}

	return wrongMessage;
}

void SceneMng::ProcessMessage( Scene::Result message, int &refUpdateCount, int &refLoopIndex )
{
	if ( !ValidateMessage( message ) )
	{
		_ASSERT_EXPR( 0, L"Error: The passed message is wrong !" );

		message = ApplyFailSafe( message );
	}

	// Attention to order of process message.
	// ex) [pop_front() -> push_front()] [push_front() -> pop_front]

	if ( message.HasRequest( Scene::Request::REMOVE_ME ) )
	{
		PopScene( /* isFront = */ true );
	}

	if ( message.HasRequest( Scene::Request::REMOVE_ALL ) )
	{
		PopAll();
	}

	if ( message.HasRequest( Scene::Request::ADD_SCENE ) )
	{
		PushScene( message.sceneType, /* isFront = */ true );
	}
	
	if ( message.HasRequest( Scene::Request::APPEND_SCENE ) )
	{
		PushScene( message.sceneType, /* isFront = */ false );
	}
	
	if ( message.HasRequest( Scene::Request::UPDATE_NEXT ) )
	{
		if ( message.HasRequest( Scene::Request::REMOVE_ME ) )
		{
			refLoopIndex--;
			refLoopIndex = std::max( -1, refLoopIndex );
			// The loop-index will be increment, so lower limit is -1.
		}
		else
		{
			refUpdateCount++;
			refUpdateCount = std::min( scast<int>( pScenes.size() ), refUpdateCount );
		}
	}
}

void SceneMng::PushScene( Scene::Type type, bool isFront )
{
	switch ( type )
	{
	case Scene::Type::Logo:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneLogo>() )
		: pScenes.push_back ( std::make_unique<SceneLogo>() );
		break;
	case Scene::Type::Title:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneTitle>() )
		: pScenes.push_back ( std::make_unique<SceneTitle>() );
		break;
	case Scene::Type::Game:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneGame>() )
		: pScenes.push_back ( std::make_unique<SceneGame>() );
		break;
	case Scene::Type::Clear:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneClear>() )
		: pScenes.push_back ( std::make_unique<SceneClear>() );
		break;
	case Scene::Type::Over:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneOver>() )
		: pScenes.push_back ( std::make_unique<SceneOver>() );
		break;
	case Scene::Type::Pause:
		( isFront )
		? pScenes.push_front( std::make_unique<ScenePause>() )
		: pScenes.push_back ( std::make_unique<ScenePause>() );
		break;
	default: _ASSERT_EXPR( 0, L"Error : The scene does not exist." ); return;
	}

	( isFront )
	? pScenes.front()->Init()
	: pScenes.back()->Init();
}

void SceneMng::PopScene( bool isFront )
{
	if ( isFront )
	{
		pScenes.front()->Uninit();
		pScenes.pop_front();
	}
	else
	{
		pScenes.back()->Uninit();
		pScenes.pop_back();
	}
}

void SceneMng::PopAll()
{
	for ( auto &it : pScenes )
	{
		it->Uninit();
	}
	pScenes.clear();
}