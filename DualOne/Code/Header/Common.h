#pragma once

#include "Donya/Vector.h"

namespace Common
{
	int		ScreenWidth();
	float	ScreenWidthF();
	long	ScreenWidthL();

	int		ScreenHeight();
	float	ScreenHeightF();
	long	ScreenHeightL();

	int		HalfScreenWidth();
	float	HalfScreenWidthF();
	long	HalfScreenWidthL();

	int		HalfScreenHeight();
	float	HalfScreenHeightF();
	long	HalfScreenHeightL();

	void	SetShowCollision( bool newState );
	void	ToggleShowCollision();
	/// <summary>
	/// If when release mode, returns false.
	/// </summary>
	bool	IsShowCollision();

	/// <summary>
	/// Returns:<para></para>
	/// True:the left-vector and right-vector is intersected. the result pointer will be set intersection-point.<para></para>
	/// False:the left-vector and right-vector is not intersected. the result pointer will be set zero.
	/// </summary>
	bool	CalcIntersectionPoint
	(
		const Donya::Vector2 &leftStart,
		const Donya::Vector2 &leftVector,
		const Donya::Vector2 &rightStart,
		const Donya::Vector2 &rightVector,
		Donya::Vector2 *pResultPoint
	);
}
