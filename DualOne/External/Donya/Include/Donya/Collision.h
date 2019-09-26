#ifndef INCLUDED_COLLISION_H_
#define INCLUDED_COLLISION_H_

#include <cstdint> // use for std::uint32_t

#include "cereal/cereal.hpp"

class Circle;

/// <summary>
/// HitBox<para></para>
/// float cx, cy : Position of center. I recommend to setting world space.<para></para>
/// float w, h : Specify half size.<para></para>
/// bool exist : Is enable collision ?
/// </summary>
class Box
{
public:
	float	cx;		// Position of center.
	float	cy;		// Position of center.
	float	w;		// Half-width.
	float	h;		// Half-height.
	bool	exist;	// Is enable collision ?
public:
	Box() : cx( 0 ), cy( 0 ), w( 0 ), h( 0 ), exist( true ) {}
	Box
	(
		float centerX, float centerY,
		float halfWidth, float halfHeight,
		bool  isExist = true
	) :
		cx( centerX ), cy( centerY ),
		w( halfWidth ), h( halfHeight ),
		exist( isExist )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( cx ), CEREAL_NVP( cy ),
			CEREAL_NVP( w ), CEREAL_NVP( h ),
			CEREAL_NVP( exist )
		);
		if ( 1 <= version )
		{
			// archive();
		}
	}
public:
	void Set				( float centerX, float centerY, float halfWidth, float halfHeight, bool isExist = true );
public:
	static bool IsHitPoint	( const Box &L, const float &RX, const float &RY, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitPoint	( const Box &L, const float &LBoxScreenPosX, const float &LBoxScreenPosY, const float &RX, const float &RY, bool ignoreExistFlag = false );
	static bool IsHitBox	( const Box &L, const Box &R, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitBox	( const Box &L, const float &LBoxScreenPosX, const float &LBoxScreenPosY, const Box &R, const float &RBoxScreenPosX, const float &RBoxScreenPosY, bool ignoreExistFlag = false );
	static bool IsHitCircle	( const Box &L, const Circle &R, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitCircle	( const Box &L, const float &LBoxScreenPosX, const float &LBoxScreenPosY, const Circle &R, const float &RCircleScreenPosX, const float &RCircleScreenPosY, bool ignoreExistFlag = false );
public:
	static inline Box Nil()
	{
		return{ 0, 0, 0, 0, false };
	}
};


/// <summary>
/// HitBox of circle<para></para>
/// float cx, cy : Position of center. I recommend to setting world space.<para></para>
/// float radius : Specify radius.<para></para>
/// bool exist :  Is enable collision ?
/// </summary>
class Circle
{
public:
	float	cx;		// Position of center.
	float	cy;		// Position of center.
	float	radius;
	bool	exist;	// Is enable collision ?
public:
	Circle() : cx( 0 ), cy( 0 ), radius( 0 ), exist( true ) {}
	Circle
	(
		float centerX, float centerY,
		float rad,
		bool  isExist = true
	) :
		cx( centerX ), cy( centerY ),
		radius( rad ),
		exist( isExist )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( cx ), CEREAL_NVP( cy ),
			CEREAL_NVP( radius ),
			CEREAL_NVP( exist )
		);
		if ( 1 <= version )
		{
			// archive();
		}
	}
public:
	void Set				( float centerX, float centerY, float rad, bool isExist = true );
public:
	static bool IsHitPoint	( const Circle &L, const float &RX, const float &RY, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitPoint	( const Circle &L, const float &LCircleScreenPosX, const float &LCircleScreenPosY, const float &RX, const float &RY, bool ignoreExistFlag = false );
	static bool IsHitCircle	( const Circle &L, const Circle &R, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitCircle	( const Circle &L, const float &LCircleScreenPosX, const float &LCircleScreenPosY, const Circle &R, const float &RCircleScreenPosX, const float &RCircleScreenPosY, bool ignoreExistFlag = false );
	static bool IsHitBox	( const Circle &L, const Box &R, bool ignoreExistFlag = false );
	/// <summary>
	/// The "ScreenPos"s are add to position.
	/// </summary>
	static bool IsHitBox	( const Circle &L, const float &LCircleScreenPosX, const float &LCircleScreenPosY, const Box &R, const float &RBoxScreenPosX, const float &RBoxScreenPosY, bool ignoreExistFlag = false );
public:
	static inline Circle Nil()
	{
		return Circle{ 0, 0, 0, false };
	}
};

bool		operator == ( const Box &L, const Box &R );
static bool	operator != ( const Box &L, const Box &R ) { return !( L == R ); }

bool		operator == ( const Circle &L, const Circle &R );
static bool	operator != ( const Circle &L, const Circle &R ) { return !( L == R ); }

CEREAL_CLASS_VERSION( Box, 1 );
CEREAL_CLASS_VERSION( Circle, 1 );


#endif // INCLUDED_COLLISION_H_
