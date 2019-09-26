#pragma once

#include <memory>

namespace Donya
{

	class Random
	{
		// TODO:I want Impl :
		// http://marupeke296.com/TIPS_No18_Randoms.html
		// http://marupeke296.com/TIPS_No16_flaotrandom.html

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	private:
		Random();
	public:
		virtual ~Random();
		Random( const Random & ) = delete;
		Random( Random && ) = delete;
		Random & operator = ( const Random & ) = delete;
		Random & operator = ( Random && ) = delete;
	public:
		inline static Random *GetInstance()
		{
			static Random instance;
			return &instance;
		}
	public:
		unsigned int	_Int	()							const; // returns 0.0f ~ std::random_device::max()
		unsigned int	_Int	( int max )					const; // returns 0.0f ~ ( max - 1 )
		int				_Int	( int min, int max )		const; // returns min  ~ ( max - 1 )
		float			_Float	()							const; // returns 0.0f ~ 1.0f
		float			_Float	( float max )				const; // returns 0.0f ~ 1.0f
		float			_Float	( float min, float max )	const; // returns 0.0f ~ 1.0f
	public:
		inline static unsigned int	GenerateInt()
		{
			return GetInstance()->_Int();
		}
		inline static unsigned int	GenerateInt( int max )
		{
			return GetInstance()->_Int( max );
		}
		inline static int			GenerateInt( int min, int max )
		{
			return GetInstance()->_Int( min, max );
		}
		inline static float			GenerateFloat()
		{
			return GetInstance()->_Float();
		}
		inline static float			GenerateFloat( float max )
		{
			return GetInstance()->_Float( max );
		}
		inline static float			GenerateFloat( float min, float max )
		{
			return GetInstance()->_Float( min, max );
		}
	private:
		template<typename ReturnType>
		inline static ReturnType	Choose() { return ReturnType(); }

		template<typename ReturnType, typename First, typename... Rest>
		inline static ReturnType	Choose( size_t randIndex, const First &first, const Rest &... rest )
		{
			return ( !randIndex ) ? first : GetInstance()->Choose<ReturnType>( randIndex - 1, rest... );
		}
	public:
	};

	template<typename ReturnType, typename First, typename... Rest>
	inline ReturnType Choose( const First &first, const Rest &... rest )
	{
		size_t randIndex = Random::GenerateInt( sizeof...( rest ) );
		return ( !randIndex ) ? first : Random::Choose<ReturnType>( randIndex - 1, rest... );
	}
}
