#pragma once

#include <string>

#include "Donya/Serializer.h"

/// <summary>
/// Measure elapsed time. this class can count until 99min-59sec-59ms with frame.
/// </summary>
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

	void Update();

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

	/// <summary>
	/// Returns string is "XX:XX:XX", min:sec:ms.
	/// </summary>
	std::string ToStr( bool isInsertColon = true );
};

static bool operator <  ( const Timer &L, const Timer &R )
{
	if ( L.Minute()  < R.Minute()  ) { return true;  }
	if ( L.Minute()  > R.Minute()  ) { return false; }
	// else
	if ( L.Second()  < R.Second()  ) { return true;  }
	if ( L.Second()  > R.Second()  ) { return false; }
	// else
	if ( L.Current() < R.Current() ) { return true;  }
	// else
	return false;
}
static bool operator >  ( const Timer &L, const Timer &R ) { return R < L; }
static bool operator <= ( const Timer &L, const Timer &R ) { return !( R < L ); }
static bool operator >= ( const Timer &L, const Timer &R ) { return !( L < R ); }

static bool operator == ( const Timer &L, const Timer &R ) { return !( L < R ) && !( R > L ); }
static bool operator != ( const Timer &L, const Timer &R ) { return !( L == R ); }

CEREAL_CLASS_VERSION( Timer, 0 )
