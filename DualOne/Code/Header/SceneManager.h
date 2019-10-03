#ifndef _INCLUDED_SCENE_MANAGER_H_
#define _INCLUDED_SCENE_MANAGER_H_

#include <memory>
#include <list>

#include "Scene.h"	// Use Scene::Result, Scene::Type.

/// <summary>
/// You must call Init() when create.
/// </summary>
class SceneMng
{
private:
	std::list<std::unique_ptr<Scene>>	pScenes;
public:
	SceneMng();
	~SceneMng();
	SceneMng( const SceneMng &  ) = delete;
	SceneMng( const SceneMng && ) = delete;
	SceneMng & operator = ( const SceneMng &  ) = delete;
	SceneMng & operator = ( const SceneMng && ) = delete;
public:
	void Init( Scene::Type initScene = Scene::Type::Logo );
	void Uninit();

	void Update( float elapsedTime );

	void Draw( float elapsedTime );
private:
	bool WillEmptyIfApplied( Scene::Result message ) const;
	/// <summary>
	/// Returns false if the message is wrong.
	/// </summary>
	bool ValidateMessage( Scene::Result message ) const;
	Scene::Result ApplyFailSafe( Scene::Result wrongMessage ) const;

	void ProcessMessage( Scene::Result message );

	/// <summary>
	/// Also doing scene initialize.
	/// </summary>
	void PushScene( Scene::Type type, bool isFront );
	void PopScene( bool isFront );

	void PopAll();
};

#endif // _INCLUDED_SCENE_MANAGER_H_