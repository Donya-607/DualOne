#pragma once

class Scene
{
public:
	enum class Request : int
	{
		NONE				= 0,
		ADD_SCENE			= 1 << 0,
		REMOVE_ME			= 1 << 1,
		REMOVE_ALL			= 1 << 2,
		ASSIGN				= REMOVE_ALL | ADD_SCENE,	// Doing REMOVE_ALL and ADD_SCENE.
	};
	enum class Type : int
	{
		Null = 0,
		Logo,
		Title,
		Game,
		Clear,
		Pause,
	};
	struct Result
	{
		Request	request		= Request::NONE;
		Type	sceneType	= Type::Null;
	public:
		Result() : request( Request::NONE ), sceneType( Type::Null ) {}
		Result( Request request, Type type ) : request( request ), sceneType( type ) {}
	public:
		void AddRequest( const Request &flag )
		{
			request = static_cast<Request>( static_cast<int>( request ) | static_cast<int>( flag ) );
		}
		void AddRequest( const Request &L, const Request &R )
		{
			AddRequest( L );
			AddRequest( R );
		}
	public:
		bool HasRequest( Request kind ) const
		{
			return
			( static_cast<int>( request ) & static_cast<int>( kind ) )
			? true
			: false;
		}
	};
public:
	Scene() {}
	virtual ~Scene() {}
public:
	virtual void	Init()			= 0;
	virtual void	Uninit()		= 0;
	virtual Result	Update( float elapsedTime )	= 0;
	virtual void	Draw( float elapsedTime )	= 0;
};
