#pragma once

#include "Donya/Serializer.h"

class Timer
{
private:
	int current;	// 0 ~ 59.
	int second;		// 0 ~ 59.
	int minute;		// 0 ~ 99.
public:
	Timer() : current( 59 ), second( 59 ), minute( 99 ) {}
	Timer( const Timer & ) = default;
	Timer &operator = ( const Timer & ) = default;
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( current ),
			CEREAL_NVP( second  ),
			CEREAL_NVP( minute  )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP() );
		}
	}
	static constexpr const char *SERIAL_ID = "Timer";
public:
	/// <summary>
	/// If setting -1, that parameter is not change.
	/// </summary>
	void Set( int newMinute = -1, int newSecond = -1, int newCurrent = -1 )
	{
		if ( newMinute  != -1 ) { minute  = newMinute;  }
		if ( newSecond  != -1 ) { second  = newSecond;  }
		if ( newCurrent != -1 ) { current = newCurrent; }
	}

	void Update()
	{
		if ( IsMaxCount() ) { return; }
		// else

		current++;

		if ( 60 <= current )
		{
			current = 0;
			second++;
			if ( 60 <= second )
			{
				auto Min = []( int lhs, int rhs )
				{
					return ( lhs < rhs ) ? lhs : rhs;
				};

				second = 0;
				minute++;
				minute = Min( 99, minute );
			}
		}
	}

	bool IsMaxCount() const
	{
		if ( minute  != 99 ) { return false; }
		if ( second  != 59 ) { return false; }
		if ( current != 59 ) { return false; }
		return true;
	}

	int Current() const { return current; }
	int Second()  const { return second;  }
	int Minute()  const { return minute;  }
};

static bool operator <  ( const Timer &L, const Timer &R )
{
	if ( R.Minute()  <  L.Minute()  ) { return false; }
	if ( R.Second()  <  L.Second()  ) { return false; }
	if ( R.Current() <= L.Current() ) { return false; }
	return true;
}
static bool operator >  ( const Timer &L, const Timer &R ) { return R < L; }
static bool operator <= ( const Timer &L, const Timer &R ) { return !( R < L ); }
static bool operator >= ( const Timer &L, const Timer &R ) { return !( L < R ); }

static bool operator == ( const Timer &L, const Timer &R ) { return !( L < R ) && !( R > L ); }
static bool operator != ( const Timer &L, const Timer &R ) { return !( L == R ); }

CEREAL_CLASS_VERSION( Timer, 0 )
