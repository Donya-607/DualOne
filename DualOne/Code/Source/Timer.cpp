#include "Timer.h"

#include <algorithm>		// use std::min().
#include <vector>

#include "Donya/Useful.h"	// Use SeparateDigits().

#undef max
#undef min

void Timer::Update()
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
			second = 0;
			minute++;
			minute = std::min( 99, minute );
		}
	}
}

/// <summary>
/// Returns string is "XX:XX:XX", min:sec:ms.
/// </summary>
std::string Timer::ToStr( bool isInsertColon )
{
	constexpr int    DIGIT = 2;
	constexpr size_t SIZE = 3;
	const std::vector<int> SEPARATED_TIMES // [0:current][1:second][2:minute]
	{
		Current(),
		Second(),
		Minute()
	};

	std::vector<int> separatedNumbers{};

	for ( size_t i = 0; i < SIZE; ++i )
	{
		auto result = Donya::SeparateDigits( SEPARATED_TIMES[i], DIGIT );
		for ( const auto &it : result )
		{
			separatedNumbers.emplace_back( it );
		}
	}

	std::string reverseNumberStr{}; // Store "123456" to "65:43:21".

	const size_t numberCount = separatedNumbers.size();
	for ( size_t i = 0; i < numberCount; ++i )
	{
		reverseNumberStr += std::to_string( separatedNumbers[i] );

		bool canInsert = ( isInsertColon && i + 1 < numberCount );
		if ( i % DIGIT == 1 && canInsert )
		{
			reverseNumberStr += ":";
		}
	}

	std::reverse( reverseNumberStr.begin(), reverseNumberStr.end() );
	return reverseNumberStr;
}
