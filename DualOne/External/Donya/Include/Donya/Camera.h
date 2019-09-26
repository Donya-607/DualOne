#ifndef INCLUDED_DONYA_CAMERA_H_
#define INCLUDED_DONYA_CAMERA_H_

#include "Constant.h"	// Use DEBUG_MODE macro.

#include "Quaternion.h"
#include "Vector.h"

namespace DirectX
{
	struct XMFLOAT4X4;
	struct XMFLOAT3;
	struct XMMATRIX;
}

namespace Donya
{

	class Camera
	{
		static constexpr unsigned int PROGRAM_VERSION = 0;
	private:
		enum class Mode
		{
			None,			// when the left-button and wheel-button is not pressed.
			OrbitAround,	// when left-clicking.
			Pan				// when wheel-pressing, or right-clicking.
		};
		struct MouseCoord
		{
			Donya::Vector2 prev{};
			Donya::Vector2 current{};
		};
	private:
		Mode				moveMode;
		float				radius;				// same as length of vector(pos-focus).
		float				scopeAngle;			// 0-based, Radian
		float				virtualDistance;	// The distance of from my-position to virtual-screen, use for Pan() move.
		Donya::Vector2		screenSize;
		Donya::Vector2		halfScreenSize;
		Donya::Vector3		pos;
		Donya::Vector3		focus;
		Donya::Vector3		velocity;
		MouseCoord			mouse;
		Donya::Quaternion	posture;
		DirectX::XMFLOAT4X4	projection;
	public:
		Camera();
		~Camera();
	public:
		void Init( float screenWidth, float screenHeight, float scopeAngle );
		/// <summary>
		/// Set Whole-size.
		/// </summary>
		void SetScreenSize( float newScreenWidth, float newScreenHeight );
		/// <summary>
		/// Set Whole-size.
		/// </summary>
		void SetScreenSize( Donya::Vector2 newScreenSize );
		/// <summary>
		/// The position are setting to { 0.0f, 0.0f, 0.0f }.<para></para>
		/// The focus are setting to { 0.0f, 0.0f, 1.0f }.
		/// </summary>
		void SetToHomePosition( Donya::Vector3 homePosition = { 0.0f, 0.0f, 0.0f }, Donya::Vector3 homeFocus = { 0.0f, 0.0f, 1.0f } );
		/// <summary>
		/// "scopeAngle" is 0-based, radian.
		/// </summary>
		void SetScopeAngle( float scopeAngle );

		void ResetOrthographicProjection();
		void ResetPerspectiveProjection();
		DirectX::XMMATRIX SetOrthographicProjectionMatrix( float width, float height, float mostNear, float mostFar );
		/// <summary>
		/// ScopeAngle, Near, Far are used to default.
		/// </summary>
		DirectX::XMMATRIX SetPerspectiveProjectionMatrix( float aspectRatio );
		DirectX::XMMATRIX SetPerspectiveProjectionMatrix( float scopeAngle, float aspectRatio, float mostNear, float mostFar );
		DirectX::XMMATRIX CalcViewMatrix() const;
		DirectX::XMMATRIX GetProjectionMatrix() const;
		Donya::Vector3 GetPos() const { return pos; }
	public:
		void Update();
	private:
		/// <summary>
		/// Set degrees to [X:45][Y:45][Z:0].
		/// </summary>
		void ResetPosture();

		void MouseUpdate();

		void ChangeMode();

		void Move();

		void Zoom();

		void OrbitAround();

		void Pan();
		void CalcDistToVirtualScreen();
		Donya::Vector3 ToWorldPos( const Donya::Vector2 &screenPos );

	#if DEBUG_MODE

	public:
		void ShowParametersToImGui();

	#endif // DEBUG_MODE
	};
}

#endif //INCLUDED_CAMERA_H_
