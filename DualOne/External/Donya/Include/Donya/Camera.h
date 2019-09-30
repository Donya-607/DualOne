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
	/// <summary>
	/// 
	/// </summary>
	class Camera
	{
		static constexpr unsigned int PROGRAM_VERSION = 0;
	public:
		/// <summary>
		/// Store information of drive the camera.
		/// </summary>
		struct Controller
		{
			Donya::Vector3	moveVelocity{};						// Set move direction(contain speed).
			Donya::Vector3	rotation{};							// Set rotate angles(radian), each angles are used to direction of rotate(e.g. rotation.x is used to yaw-axis rotate).
			float			slerpPercent{ 1.0f };				// Set percentage of interpolation(0.0f ~ 1.0f, will be clamped). use to rotate direction to "lookAt" if that is not zero.
			bool			moveAtLocalSpace{ true };			// Specify the space of movement. world-space or local-space(with current posture).
		public:
			// This condition is same as default constructed condition.
			void SetNoOperation()
			{
				moveVelocity	= Donya::Vector3::Zero();
				rotation		= Donya::Vector3::Zero();
				slerpPercent	= 0.0f;
			}
		};
	private:
		float				focusDistance;		// This enable when distance != zero, the focus is there front of camera.
		float				scopeAngle;			// Radian
		Donya::Vector2		screenSize;
		Donya::Vector2		halfScreenSize;
		Donya::Vector3		pos;
		Donya::Vector3		focus;
		Donya::Vector3		velocity;
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

		/// <summary>
		/// Set distance of focus from camera position.<para></para>
		/// ! If the camera rotate after called this, the focus will be move also. !
		/// </summary>
		void SetFocusDistance( float distance );
		/// <summary>
		/// Set the focus position in world-space.<para></para>
		/// If that focus position same as camera position, that focus position will be pushed to front.<para></para>
		/// ! The focus position is fixed, that is not move if the camera rotate after call this.
		/// </summary>
		void SetFocusCoordinate( const Donya::Vector3 &coordinate );

		void ResetOrthographicProjection();
		/// <summary>
		/// Requirement : the camera must be already initialized.
		/// </summary>
		void ResetPerspectiveProjection();
		DirectX::XMMATRIX	SetOrthographicProjectionMatrix( float width, float height, float mostNear, float mostFar );
		/// <summary>
		/// ScopeAngle, Near, Far are used to default.
		/// </summary>
		DirectX::XMMATRIX	SetPerspectiveProjectionMatrix( float aspectRatio );
		DirectX::XMMATRIX	SetPerspectiveProjectionMatrix( float scopeAngle, float aspectRatio, float mostNear, float mostFar );
		DirectX::XMMATRIX	CalcViewMatrix() const;
		DirectX::XMMATRIX	GetProjectionMatrix() const;
		Donya::Vector3		GetPos() const { return pos; }
		Donya::Quaternion	GetPosture() const { return posture; }
	public:
		void Update( Controller controller );
	private:
		bool IsFocusFixed() const;

		void ResetPosture();

		void SetFocusToFront();

		void Move( Controller controller );

		void Rotate( Controller controller );

	#if DEBUG_MODE

	public:
		void ShowParametersToImGui();

	#endif // DEBUG_MODE
	};
}

#endif //INCLUDED_CAMERA_H_
