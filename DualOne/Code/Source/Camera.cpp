#include "Camera.h"

#include <algorithm>
#include <array>
#include <string>

#include "Donya/Constant.h"
#include "Donya/Keyboard.h"
#include "Donya/Mouse.h"
#include "Donya/Useful.h"
#include "Donya/UseImgui.h"

#undef min
#undef max

using namespace DirectX;

Camera::Camera() :
	focusDistance(),
	scopeAngle(),
	screenSize( 1920.0f, 1080.0f ), halfScreenSize(),
	pos(), focus(), velocity(),
	posture(),
	projection()
{
	/*
	constexpr float DEFAULT_FOV		= ToRadian( 30.0f );
	constexpr float DEFAULT_WIDTH	= 1920.0f;
	constexpr float DEFAULT_HEIGHT	= 1080.0f;

	SetScreenSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	SetScopeAngle( DEFAULT_FOV );
	ResetPerspectiveProjection();
	ResetPosture();
	*/
}
Camera::~Camera() = default;

void Camera::Init( float screenWidth, float screenHeight, float scopeAngle )
{
	SetScreenSize( screenWidth, screenHeight );
	SetScopeAngle( scopeAngle );
	SetToHomePosition();
	ResetPerspectiveProjection();
	ResetPosture();
}

void Camera::SetScreenSize( float newScreenWidth, float newScreenHeight )
{
	SetScreenSize( Donya::Vector2{ newScreenWidth, newScreenHeight } );
}
void Camera::SetScreenSize( Donya::Vector2 newScreenSize )
{
	screenSize		= newScreenSize;
	halfScreenSize	= screenSize * 0.5f;
}

void Camera::SetToHomePosition( Donya::Vector3 homePosition, Donya::Vector3 homeFocus )
{
	pos			= homePosition;
	focus		= homeFocus;
	posture		= Donya::Quaternion::Identity();
}

void Camera::SetScopeAngle( float scope )
{
	scopeAngle = scope;
}

void Camera::SetFocusDistance( float distance )
{
	focusDistance = distance;
	SetFocusToFront();
}
void Camera::SetFocusCoordinate( const Donya::Vector3 &coordinate )
{
	focus = coordinate;
	focusDistance = 0.0f;
}

void Camera::ResetOrthographicProjection()
{
	SetOrthographicProjectionMatrix( 16.0f, 9.0f, 0.01f, 1000.0f );
}
void Camera::ResetPerspectiveProjection()
{
	_ASSERT_EXPR( !ZeroEqual( screenSize.y ), L"Zero-divide detected!" );
	float aspectRatio	= screenSize.x / screenSize.y;
	float mostNear		= 0.1f;
	float mostFar		= 1000.0f;

	SetPerspectiveProjectionMatrix( scopeAngle, aspectRatio, mostNear, mostFar );
}

XMMATRIX Camera::SetOrthographicProjectionMatrix( float width, float height, float mostNear, float mostFar )
{
	XMStoreFloat4x4( &projection, XMMatrixOrthographicLH( width, height, mostNear, mostFar ) );
	return GetProjectionMatrix();
}
XMMATRIX Camera::SetPerspectiveProjectionMatrix( float aspectRatio )
{
	return SetPerspectiveProjectionMatrix( scopeAngle, aspectRatio, 0.1f, 1000.0f );
}
XMMATRIX Camera::SetPerspectiveProjectionMatrix( float scopeAngle, float aspectRatio, float mostNear, float mostFar )
{
	XMStoreFloat4x4( &projection, XMMatrixPerspectiveFovLH( scopeAngle, aspectRatio, mostNear, mostFar ) );
	return GetProjectionMatrix();
}

XMMATRIX Camera::CalcViewMatrix() const
{
	XMMATRIX R{};
	{
		Donya::Quaternion invRot = posture.Conjugate();
		XMFLOAT4X4 rotate = invRot.RequireRotationMatrix();

		R = XMLoadFloat4x4( &rotate );
	}

	XMMATRIX T = XMMatrixTranslation( -pos.x, -pos.y, -pos.z );

	return { T * R };
}

XMMATRIX Camera::GetProjectionMatrix() const
{
	return XMLoadFloat4x4( &projection );
}

void Camera::Update( Controller controller )
{
	Move( controller );

	Rotate( controller );
}

// Interpolate is not used in current implement.
/*
void Camera::Interpolate()
{
	constexpr size_t SIZE = 1;
	const std::array<DirectX::XMFLOAT3 *, SIZE> DEST =
	{
		&destPos,
		&destFocus
	};
	const std::array<DirectX::XMFLOAT3 *, SIZE> POS =
	{
		&pos,
		&focus
	};

	for ( size_t i = 0; i < SIZE; ++i )
	{
		DirectX::XMFLOAT3 diff = Sub( *DEST[i], *POS[i] );
		if ( fabsf( Length( diff ) ) < FLT_EPSILON )
		{
			*POS[i] = *DEST[i];
			return;
		}
		// else

		diff = Multi( diff, chaseSpeed );

		*POS[i] = Add( *POS[i], diff );
	}
}
*/

bool Camera::IsFocusFixed() const
{
	return ( ZeroEqual( focusDistance ) ) ? true : false;
}

void Camera::ResetPosture()
{
	posture = Donya::Quaternion::Make( 0.0f, 0.0f, 0.0f );
}

void Camera::SetFocusToFront()
{
	Donya::Vector3 localFront = posture.RotateVector( Donya::Vector3::Front() );

	localFront.Normalize();
	localFront *= focusDistance;

	focus = pos + localFront;
}

void Camera::Move( Controller controller )
{
	if ( controller.moveVelocity.IsZero() ) { return; }
	// else

	if ( controller.moveAtLocalSpace )
	{
		controller.moveVelocity = posture.RotateVector( controller.moveVelocity );
	}

	pos += controller.moveVelocity;
	if ( !IsFocusFixed() )
	{
		focus += controller.moveVelocity;
		SetFocusToFront();
	}
	else
	{
		// TODO:�t�H�[�J�X�n�_�֌����N�H�[�^�j�I�����Z�o���Cposture�ɋ��ʐ��`��Ԃ����������
	}
}

void Camera::Rotate( Controller controller )
{
	if ( controller.rotation.IsZero() ) { return; }
	// else

	auto GetAxis = []( const Donya::Quaternion &q, Donya::Vector3 axis, bool inLocalSpace )
	{
		if ( inLocalSpace )
		{
			axis = q.RotateVector( axis );
		}

		return axis;
	};

	Donya::Quaternion resultPosture = posture;

	if ( !ZeroEqual( controller.rotation.x ) )
	{
		Donya::Vector3 axis = GetAxis( resultPosture, Donya::Vector3::Up(), /* inLocalSpace = */ false );
		Donya::Quaternion rotate = Donya::Quaternion::Make( axis, controller.rotation.x );

		resultPosture = rotate * resultPosture;
	}
	if ( !ZeroEqual( controller.rotation.y ) )
	{
		Donya::Vector3 axis = GetAxis( resultPosture, Donya::Vector3::Right(), /* inLocalSpace = */ true );
		Donya::Quaternion rotate = Donya::Quaternion::Make( axis, controller.rotation.y );

		resultPosture = rotate * resultPosture;
	}
	if ( !ZeroEqual( controller.rotation.z ) )
	{
		Donya::Vector3 axis = GetAxis( resultPosture, Donya::Vector3::Front(), /* inLocalSpace = */ true );
		Donya::Quaternion rotate = Donya::Quaternion::Make( axis, controller.rotation.z );

		resultPosture = rotate * resultPosture;
	}

	// posture = Donya::Quaternion::Slerp( posture, resultPosture, controller.slerpPercent );
	posture = resultPosture;

	// Norm() != 1
	if ( !ZeroEqual( posture.Length() - 1.0f ) )
	{
		posture.Normalize();
	}

	SetFocusToFront();
}

#if DEBUG_MODE

void Camera::ShowParametersToImGui()
{
#if USE_IMGUI

	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"�R�c�E�J����" ) )
		{
			if ( ImGui::TreeNode( u8"������@" ) )
			{
				std::string key{ u8"\"R\" �L�[ : �ʒu�ƒ����_�����Z�b�g. " };
				std::string zoom{ u8"�}�E�X�z�C�[�� : �Y�[���C���E�A�E�g." };
				std::string rotate{ u8"���N���b�N(�{�ړ�) : �����_�����ɉ�]�ړ�." };
				std::string pan{ u8"�z�C�[���������݁i�{�ړ��j or\n�E�N���b�N(�{�ړ�) : �ʒu�ƒ����_���ړ�." };

				auto ShowString = []( const std::string &str )
				{
					ImGui::Text( str.c_str() );
				};

				ShowString( key );
				ShowString( zoom );
				ShowString( rotate );
				ShowString( pan );

				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( "Parameter" ) )
			{
				std::string vec3Info{ "[X:%5.3f][Y:%5.3f][Z:%5.3f]" };
				std::string vec4Info{ "[X:%5.3f][Y:%5.3f][Z:%5.3f][W:%5.3f]" };
				auto ShowVec3 = [&vec3Info]( std::string name, const Donya::Vector3 &param )
				{
					ImGui::Text( ( name + vec3Info ).c_str(), param.x, param.y, param.z );
				};
				auto ShowVec4 = [&vec4Info]( std::string name, const Donya::Vector4 &param )
				{
					ImGui::Text( ( name + vec4Info ).c_str(), param.x, param.y, param.z, param.w );
				};

				Donya::Vector3 up = posture.RotateVector( Donya::Vector3::Up() );
				Donya::Vector3 right = posture.RotateVector( Donya::Vector3::Right() );
				Donya::Vector3 front = posture.RotateVector( Donya::Vector3::Front() );

				ShowVec3( "Pos", pos );
				ShowVec3( "Focus", focus );
				ShowVec3( "Up", up );
				ShowVec3( "Right", right );
				ShowVec3( "Front", front );

				auto euler = posture.GetEulerAngles();
				euler.x = ToDegree( euler.x );
				euler.y = ToDegree( euler.y );
				euler.z = ToDegree( euler.z );
				ShowVec3( "Euler", euler );

				Donya::Vector4 vec4Posture
				{
					posture.x,
					posture.y,
					posture.z,
					posture.w
				};
				ShowVec4( "Posture", vec4Posture );
				ImGui::Text( "Norm[%5.3f]", posture.Length() );

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

#endif // USE_IMGUI
}

#endif // DEBUG_MODE
