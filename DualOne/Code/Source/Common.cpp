#include "Common.h"

#include "Donya/Constant.h"
#include "Donya/Useful.h"	// Use EPSILON, ZeroEqual().

namespace Common
{
	constexpr long	SCREEN_WIDTH_L	= 1920L;
	constexpr int	SCREEN_WIDTH_I	= scast<int>	( SCREEN_WIDTH_L	);
	constexpr float	SCREEN_WIDTH_F	= scast<float>	( SCREEN_WIDTH_L	);

	constexpr long	SCREEN_HEIGHT_L	= 1080L;
	constexpr int	SCREEN_HEIGHT_I	= scast<int>	( SCREEN_HEIGHT_L	);
	constexpr float	SCREEN_HEIGHT_F	= scast<float>	( SCREEN_HEIGHT_L	);

	int		ScreenWidth()			{ return SCREEN_WIDTH_I;			}
	float	ScreenWidthF()			{ return SCREEN_WIDTH_F;			}
	long	ScreenWidthL()			{ return SCREEN_WIDTH_L;			}

	int		ScreenHeight()			{ return SCREEN_HEIGHT_I;			}
	float	ScreenHeightF()			{ return SCREEN_HEIGHT_F;			}
	long	ScreenHeightL()			{ return SCREEN_HEIGHT_L;			}

	int		HalfScreenWidth()		{ return SCREEN_WIDTH_I >> 1;		}
	float	HalfScreenWidthF()		{ return SCREEN_WIDTH_F * 0.5f;		}
	long	HalfScreenWidthL()		{ return SCREEN_WIDTH_L >> 1;		}

	int		HalfScreenHeight()		{ return SCREEN_HEIGHT_I >> 1;		}
	float	HalfScreenHeightF()		{ return SCREEN_HEIGHT_F * 0.5f;	}
	long	HalfScreenHeightL()		{ return SCREEN_HEIGHT_L >> 1;		}

#if DEBUG_MODE
	static bool showCollision = false;
#endif // DEBUG_MODE
	void	SetShowCollision( bool newState )
	{
	#if DEBUG_MODE
		showCollision = newState;
	#endif // DEBUG_MODE
	}
	void	ToggleShowCollision()
	{
	#if DEBUG_MODE
		showCollision = !showCollision;
	#endif // DEBUG_MODE
	}
	bool	IsShowCollision()
	{
	#if DEBUG_MODE
		return showCollision;
	#else
		return false;
	#endif // DEBUG_MODE
	}

	bool	CalcIntersectionPoint( const Donya::Vector2 &LStart, const Donya::Vector2 &LVec, const Donya::Vector2 &RStart, const Donya::Vector2 &RVec, Donya::Vector2 *pResult )
	{
		// From http://marupeke296.com/COL_2D_No10_SegmentAndSegment.html

		float crsLR = Donya::Vector2::Cross( LVec, RVec );
		if ( ZeroEqual( crsLR ) ) { return false; }
		// else

		Donya::Vector2 vecSS = RStart - LStart;
		float crsSL = Donya::Vector2::Cross( vecSS, LVec );
		float crsSR = Donya::Vector2::Cross( vecSS, RVec );

		float internDivL = crsSR / crsLR;
		float internDivR = crsSL / crsLR;

		auto WithinZeroOne = []( float value )
		{
			return ( value < 0.0f || 1.0f < value ) ? false : true;
		};
		if ( !WithinZeroOne( internDivL ) || !WithinZeroOne( internDivR ) ) { return false; }

		if ( pResult )
		{
			*pResult = LStart + ( LVec * internDivL );
		}

		return true;
	}
}
