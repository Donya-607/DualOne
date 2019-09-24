#include "SceneManager.h"

#include "Donya/Resource.h"

#include "Common.h"
#include "Fader.h"
#include "Scene.h"
#include "SceneGame.h"
#include "SceneLogo.h"
#include "SceneTitle.h"
#include "ScenePause.h"

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

#if UPDATE_ALL_STACKED_SCENE

	for ( size_t i = 0; i < pScenes.size(); ++i )
	{
		message = ( *std::next( pScenes.begin(), i ) )->Update( elapsedTime );

		ProcessMessage( message );
	}
#else

	message = ( *pScenes.begin() )->Update( elapsedTime );
	ProcessMessage( message );

#endif // UPDATE_ALL_STACKED_SCENE

	Fader::Get().Update();
}

void SceneMng::Draw( float elapsedTime )
{
	const auto &end = pScenes.crend();
	for ( auto it   = pScenes.crbegin(); it != end; ++it )
	{
		( *it )->Draw( elapsedTime );
	}

	Fader::Get().Draw();
}

void SceneMng::ProcessMessage( Scene::Result message )
{
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
	case Scene::Type::Pause:
		( isFront )
		? pScenes.push_front( std::make_unique<SceneGame>() )
		: pScenes.push_back ( std::make_unique<SceneGame>() );
		break;
	//case Scene::Type::Clear:
	//	( isFront )
	//	? pScenes.push_front( std::make_unique<SceneClear>() )
	//	: pScenes.push_back ( std::make_unique<SceneClear>() );
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