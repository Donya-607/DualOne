#include "Framework.h"

#include <array>

#include "Donya/Blend.h"
#include "Donya/Constant.h"
#include "Donya/Donya.h"
#include "Donya/Keyboard.h"
#include "Donya/Mouse.h"
#include "Donya/Resource.h"
#include "Donya/ScreenShake.h"
#include "Donya/Sound.h"
#include "Donya/Useful.h"
#include "Donya/UseImgui.h"

#include "Common.h"
#include "Music.h"

using namespace DirectX;

Framework::Framework() :
	pSceneMng( nullptr )
{}
Framework::~Framework() = default;

bool Framework::Init()
{
	LoadSounds();

	pSceneMng = std::make_unique<SceneMng>();

#if DEBUG_MODE
	pSceneMng->Init( Scene::Type::Game );
#else
	pSceneMng->Init( Scene::Type::Logo );
#endif // DEBUG_MODE

	return true;
}

void Framework::Uninit()
{
	pSceneMng->Uninit();
}

void Framework::Update( float elapsedTime/*Elapsed seconds from last frame*/ )
{
#if DEBUG_MODE

	if ( Donya::Keyboard::Trigger( 'C' ) )
	{
		char breakPoint = 0;
	}
	if ( Donya::Keyboard::Trigger( 'T' ) )
	{
		Donya::ToggleShowStateOfImGui();
	}
	if ( Donya::Keyboard::Trigger( 'H' ) )
	{
		Common::ToggleShowCollision();
	}

	DebugShowInformation();

#endif // DEBUG_MODE

	pSceneMng->Update( elapsedTime );
}

#define ENABLE_3D_TEST ( true && DEBUG_MODE && USE_IMGUI )
#if ENABLE_3D_TEST
#include <memory>
#include "Donya/GeometricPrimitive.h"
#include "Donya/Loader.h"
#include "Donya/StaticMesh.h"
#include "Donya/Useful.h"
#include "Camera.h"
#include "FilePath.h"
#endif // ENABLE_3D_TEST
void Framework::Draw( float elapsedTime/*Elapsed seconds from last frame*/ )
{
	Donya::Blend::Set( Donya::Blend::Mode::ALPHA );

	pSceneMng->Draw( elapsedTime );

#if ENABLE_3D_TEST

	auto Draw3DModelTest = [&]()
	{
		auto InitializedCameraPointer = []()
		{
			std::shared_ptr<Camera> pCamera = std::make_shared<Camera>();
			pCamera->Init
			(
				Common::ScreenWidthF(),
				Common::ScreenHeightF(),
				ToRadian( 30.0f ) // FOV
			);
			pCamera->SetFocusDistance( 2.0f );
			return pCamera;
		};
		static std::shared_ptr<Camera> pCamera = InitializedCameraPointer();
	
		auto MakeControlStructWithMouse = []()
		{
			static Donya::Int2 prevMouse{};
			static Donya::Int2 currMouse{};

			prevMouse = currMouse;

			auto nowMouse = Donya::Mouse::Coordinate();
			currMouse.x = scast<float>( nowMouse.x );
			currMouse.y = scast<float>( nowMouse.y );

			auto IsEqual = []( const Donya::Int2 &lhs, const Donya::Int2 &rhs )
			{
				if ( lhs.x != rhs.x ) { return false; }
				if ( lhs.y != rhs.y ) { return false; }
				return true;
			};
			bool isInputMouseButton = Donya::Mouse::Press( Donya::Mouse::Kind::LEFT ) || Donya::Mouse::Press( Donya::Mouse::Kind::MIDDLE ) || Donya::Mouse::Press( Donya::Mouse::Kind::RIGHT );
			bool isDriveMouse = ( !IsEqual( prevMouse, currMouse ) ) || Donya::Mouse::WheelRot() || isInputMouseButton;
			if ( !isDriveMouse )
			{
				Camera::Controller noop{};
				noop.SetNoOperation();
				return noop;
			}

			Donya::Vector3 diff{};
			{
				Donya::Vector2 vec2 = ( currMouse - prevMouse ).Float();

				diff.x = vec2.x;
				diff.y = vec2.y * -1.0f;
			}

			Donya::Vector3 movement{};
			Donya::Vector3 rotation{};

			if ( Donya::Mouse::Press( Donya::Mouse::Kind::LEFT ) )
			{
				constexpr float ROT_AMOUNT = ToRadian( 1.0f );
				rotation.x = diff.x * ROT_AMOUNT;
				rotation.y = diff.y * ROT_AMOUNT;
			}
			else
			if ( Donya::Mouse::Press( Donya::Mouse::Kind::MIDDLE ) )
			{
				constexpr float MOVE_SPEED = 0.1f;
				movement.x = diff.x * MOVE_SPEED;
				movement.y = diff.y * MOVE_SPEED;
			}

			constexpr float FRONT_SPEED = 3.5f;
			movement.z = FRONT_SPEED * scast<float>( Donya::Mouse::WheelRot() );

			Donya::Quaternion rotYaw	= Donya::Quaternion::Make( Donya::Vector3::Up(), rotation.x );

			Donya::Vector3 right = Donya::Vector3::Right();
			right = rotYaw.RotateVector( right );
			Donya::Quaternion rotPitch	= Donya::Quaternion::Make( right, rotation.y );

			Donya::Quaternion rotQ		= rotYaw * rotPitch;

			static Donya::Vector3 front = Donya::Vector3::Front();

			if ( !rotation.IsZero() )
			{
				front = rotQ.RotateVector( front );
				front.Normalize();
			}

			Camera::Controller ctrl{};
			ctrl.moveVelocity		= movement;
			ctrl.rotation			= rotation;
			ctrl.slerpPercent		= 1.0f;
			ctrl.moveAtLocalSpace	= true;

			return ctrl;
		};
		pCamera->Update( MakeControlStructWithMouse() );
		if ( Donya::Keyboard::Trigger( 'R' ) )
		{
			pCamera->SetToHomePosition();
		}

		XMMATRIX W_Cube{};
		XMMATRIX W_Sphere{};
		XMMATRIX W_TexBoard{};
		XMMATRIX W_Mesh{};
		{
			static float scale = 1.0f; // 0.1f;
			static float angleX = 0.0f; // -10.0f;
			static float angleY = 0.0f; // -160.0f;
			static float angleZ = 0.0f; // 0;
			static float moveX = 0.0f; // 2.0f;
			static float moveY = 0.0f; // -2.0f;
			static float moveZ = 5.0f; // 0;

			if ( 0 )
			{
				constexpr float SCALE_ADD = 0.01f;
				constexpr float ANGLE_ADD = 1.0f;
				constexpr float MOVE_ADD = 0.1f;

				if ( Donya::Keyboard::Press( 'W'		) ) { scale  += SCALE_ADD; }
				if ( Donya::Keyboard::Press( 'S'		) ) { scale  -= SCALE_ADD; }
				if ( Donya::Keyboard::Press( VK_UP		) ) { angleX += ANGLE_ADD; }
				if ( Donya::Keyboard::Press( VK_DOWN	) ) { angleX -= ANGLE_ADD; }
				if ( Donya::Keyboard::Press( VK_LEFT	) ) { angleY += ANGLE_ADD; }
				if ( Donya::Keyboard::Press( VK_RIGHT	) ) { angleY -= ANGLE_ADD; }
				if ( Donya::Keyboard::Press( 'A'		) ) { angleZ += ANGLE_ADD; }
				if ( Donya::Keyboard::Press( 'D'		) ) { angleZ -= ANGLE_ADD; }
				if ( Donya::Keyboard::Press( 'I'		) ) { moveY  += MOVE_ADD;  }
				if ( Donya::Keyboard::Press( 'K'		) ) { moveY  -= MOVE_ADD;  }
				if ( Donya::Keyboard::Press( 'L'		) ) { moveX  += MOVE_ADD;  }
				if ( Donya::Keyboard::Press( 'J'		) ) { moveX  -= MOVE_ADD;  }
			}

			XMMATRIX S	= XMMatrixScaling( scale, scale, scale );
			XMMATRIX RX	= XMMatrixRotationX( ToRadian( angleX ) );
			XMMATRIX RY	= XMMatrixRotationY( ToRadian( angleY ) );
			XMMATRIX RZ	= XMMatrixRotationZ( ToRadian( angleZ ) );
			XMMATRIX R	= ( RZ * RY ) * RX;
			constexpr float SHIFT = 1.0f;
			XMMATRIX T_Cube		= XMMatrixTranslation( moveX - SHIFT, moveY, moveZ );
			XMMATRIX T_Sphere	= XMMatrixTranslation( moveX + SHIFT, moveY, moveZ );
			XMMATRIX T_TexBoard	= XMMatrixTranslation( moveX, moveY, moveZ );

			W_Cube		= S * R * T_Cube;
			W_Sphere	= S * R * T_Sphere;
			W_TexBoard	= S * R * T_TexBoard;
			W_Mesh		= W_TexBoard;
		}

		XMMATRIX V = pCamera->CalcViewMatrix();

		XMFLOAT4X4 worldViewProjection_Cube{};
		XMFLOAT4X4 worldViewProjection_Sphere{};
		XMFLOAT4X4 worldViewProjection_TexBoard{};
		XMFLOAT4X4 worldViewProjection_Mesh{};
		{
			XMMATRIX P = pCamera->GetProjectionMatrix();

			XMStoreFloat4x4
			(
				&worldViewProjection_Cube,
				XMMatrixMultiply( W_Cube, XMMatrixMultiply( V, P ) )
			);
			XMStoreFloat4x4
			(
				&worldViewProjection_Sphere,
				XMMatrixMultiply( W_Sphere, XMMatrixMultiply( V, P ) )
			);
			XMStoreFloat4x4
			(
				&worldViewProjection_TexBoard,
				XMMatrixMultiply( W_TexBoard, XMMatrixMultiply( V, P ) )
			);

			worldViewProjection_Mesh = worldViewProjection_TexBoard;
		}

		XMFLOAT4X4 world_Cube{};
		XMFLOAT4X4 world_Sphere{};
		XMFLOAT4X4 world_TexBoard{};
		XMFLOAT4X4 world_Mesh{};
		XMStoreFloat4x4( &world_Cube, W_Cube );
		XMStoreFloat4x4( &world_Sphere, W_Sphere );
		XMStoreFloat4x4( &world_TexBoard, W_TexBoard );
		world_Mesh = world_TexBoard;

		XMFLOAT4 cameraPos{};
		{
			XMFLOAT3 ref = pCamera->GetPos();
			cameraPos.x = ref.x;
			cameraPos.y = ref.y;
			cameraPos.z = ref.z;
			cameraPos.w = 1.0f;
		}

		static XMFLOAT4 lightDirection{ 0.0f, 0.0f, 1.0f, 0.0f };
		static XMFLOAT4 mtlColor{ 0.4f, 1.0f, 0.7f, 1.0f };
		int drawCount = 1;
	#if USE_IMGUI
		pCamera->ShowParametersToImGui();
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( u8"ÇRÇcÅEÉeÉXÉg" ) )
			{
				ImGui::SliderFloat3( "Light Direction", &lightDirection.x, -1.0f, 1.0f );
				ImGui::ColorEdit4( "Material Color", &mtlColor.x );
				ImGui::SliderInt( u8"ï`âÊâÒêî", &drawCount, 0, 1024 );

				ImGui::TreePop();
			}

			ImGui::End();
		}
	#endif // USE_IMGUI
	
		auto InitializedCube = []()
		{
			Donya::Geometric::Cube cube{};
			cube.Init();
			return cube;
		};
		auto InitializedSphere = []()
		{
			Donya::Geometric::Sphere sphere{};
			sphere.Init();
			return sphere;
		};
		auto InitializedTexBoard = []()
		{
			Donya::Geometric::TextureBoard texBoard{ L"./Data/Images/Rights/FMOD Logo White - Black Background.png" };
			texBoard.Init();
			return texBoard;
		};
		auto InitializedStaticMesh = []( std::string fullPath )
		{
			Donya::Loader loader{};
			loader.Load( fullPath, nullptr );

			return Donya::StaticMesh::Create( loader );
		};
		static Donya::Geometric::Cube				cube			= InitializedCube();
		static Donya::Geometric::Sphere				sphere			= InitializedSphere();
		static Donya::Geometric::TextureBoard		texBoard		= InitializedTexBoard();
		for ( int i = 0; i < drawCount; ++i )
		{
			cube.Render( worldViewProjection_Cube, world_Cube, lightDirection, mtlColor );
			sphere.Render( worldViewProjection_Sphere, world_Sphere, lightDirection, mtlColor );
			texBoard.Render( worldViewProjection_TexBoard, world_TexBoard, lightDirection, mtlColor );

			if ( pStaticMeshFBX )
			{
				pStaticMeshFBX->Render( worldViewProjection_Mesh, world_Mesh, lightDirection, mtlColor, cameraPos );
			}
		}
	};
	Draw3DModelTest();

#endif // ENABLE_3D_TEST
}

#undef ENABLE_3D_TEST

bool Framework::LoadSounds()
{
	using Donya::Sound::Load;
	using Music::ID;

	struct Bundle
	{
		ID id;
		std::string fileName;
		bool isEnableLoop;
	public:
		Bundle() : id(), fileName(), isEnableLoop( false ) {}
		Bundle( ID id, const char *fileName, bool isEnableLoop ) : id( id ), fileName( fileName ), isEnableLoop( isEnableLoop ) {}
		Bundle( ID id, const std::string &fileName, bool isEnableLoop ) : id( id ), fileName( fileName ), isEnableLoop( isEnableLoop ) {}
	};

	const std::array<Bundle, ID::TERMINATION_OF_MUSIC_ID> bandles =
	{
		{	// ID, FilePath, isEnableLoop
			{ ID::BGM_Title,		"./Data/Sounds/BGM/Title.mp3",				true  },
			{ ID::BGM_Game,			"./Data/Sounds/BGM/Game.mp3",				true  },

			{ ID::ItemChoose,		"./Data/Sounds/SE/UI/ChooseItem.wav",		false },
			{ ID::ItemDecision,		"./Data/Sounds/SE/UI/DecisionItem.wav",		false },
		},
	};

	bool result = true, successed = true;

	for ( size_t i = 0; i < ID::TERMINATION_OF_MUSIC_ID; ++i )
	{
		result = Load( bandles[i].id, bandles[i].fileName.c_str(), bandles[i].isEnableLoop );
		if ( !result ) { successed = false; }
	}

	return successed;
}

#if DEBUG_MODE && USE_IMGUI
#include "Donya/Easing.h"
#endif // DEBUG_MODE && USE_IMGUI
void Framework::DebugShowInformation()
{
#if DEBUG_MODE && USE_IMGUI

	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( "Information" ) )
		{
			int x = 0, y = 0;
			Donya::Mouse::Coordinate( &x, &y );
			ImGui::Text( "Mouse[X:%d][Y:%d]", x, y );
			ImGui::Text( "Wheel[%d]", Donya::Mouse::WheelRot() );

			int LB = 0, MB = 0, RB = 0;
			LB = Donya::Mouse::Press( Donya::Mouse::LEFT );
			MB = Donya::Mouse::Press( Donya::Mouse::MIDDLE );
			RB = Donya::Mouse::Press( Donya::Mouse::RIGHT );
			ImGui::Text( "LB : %d, MB : %d, RB : %d", LB, MB, RB );

			ImGui::TreePop();
		}

		if ( ImGui::TreeNode( "Easing Test" ) )
		{
			using namespace Donya::Easing;

			static float time		= 0.0f;
			ImGui::SliderFloat( u8"éûä‘", &time, 0.0f, 1.0f );
			ImGui::Text( "" );
			static Donya::Easing::Type type = Donya::Easing::Type::In;
			{
				int iType = scast<int>( type );

				std::string caption = "Type : ";
				if ( type == Donya::Easing::Type::In ) { caption += "In"; }
				if ( type == Donya::Easing::Type::Out ) { caption += "Out"; }
				if ( type == Donya::Easing::Type::InOut ) { caption += "InOut"; }

				ImGui::SliderInt( caption.c_str(), &iType, 0, 2 );

				type = scast<Donya::Easing::Type>( iType );
			}

			constexpr unsigned int SIZE = scast<unsigned int>( Kind::ENUM_TERMINATION );
			constexpr std::array<Kind, SIZE> KINDS
			{
				Kind::Linear,
				Kind::Back,
				Kind::Bounce,
				Kind::Circular,
				Kind::Cubic,
				Kind::Elastic,
				Kind::Exponential,
				Kind::Quadratic,
				Kind::Quartic,
				Kind::Quintic,
				Kind::Sinusoidal,
				Kind::Smooth,
				Kind::SoftBack,
				Kind::Step,
			};
			std::array<float, SIZE> RESULTS
			{
				Ease( KINDS[ 0],	type,	time ),
				Ease( KINDS[ 1],	type,	time ),
				Ease( KINDS[ 2],	type,	time ),
				Ease( KINDS[ 3],	type,	time ),
				Ease( KINDS[ 4],	type,	time ),
				Ease( KINDS[ 5],	type,	time ),
				Ease( KINDS[ 6],	type,	time ),
				Ease( KINDS[ 7],	type,	time ),
				Ease( KINDS[ 8],	type,	time ),
				Ease( KINDS[ 9],	type,	time ),
				Ease( KINDS[10],	type,	time ),
				Ease( KINDS[11],	type,	time ),
				Ease( KINDS[12],	type,	time ),
				Ease( KINDS[13],	type,	time ),
			};

			auto MakeCaption = [&]( Kind kind )->std::string
			{
				std::string rv{};
				switch ( kind )
				{
				case Donya::Easing::Kind::Linear:
					rv = "Linear"; break;
				case Donya::Easing::Kind::Back:
					rv = "Back"; break;
				case Donya::Easing::Kind::Bounce:
					rv = "Bounce"; break;
				case Donya::Easing::Kind::Circular:
					rv = "Circular"; break;
				case Donya::Easing::Kind::Cubic:
					rv = "Cubic"; break;
				case Donya::Easing::Kind::Elastic:
					rv = "Elastic"; break;
				case Donya::Easing::Kind::Exponential:
					rv = "Exponential"; break;
				case Donya::Easing::Kind::Quadratic:
					rv = "Quadratic"; break;
				case Donya::Easing::Kind::Quartic:
					rv = "Quartic"; break;
				case Donya::Easing::Kind::Quintic:
					rv = "Quintic"; break;
				case Donya::Easing::Kind::Smooth:
					rv = "Smooth"; break;
				case Donya::Easing::Kind::Sinusoidal:
					rv = "Sinusoidal"; break;
				case Donya::Easing::Kind::SoftBack:
					rv = "SoftBack"; break;
				case Donya::Easing::Kind::Step:
					rv = "Step"; break;
				default:
					rv = "Error Type"; break;
				}

				return rv;
			};

			for ( unsigned int i = 0; i < SIZE; ++i )
			{
				float result = RESULTS[i];
				std::string caption = MakeCaption( KINDS[i] );
				ImGui::SliderFloat( caption.c_str(), &result, 0.0f, 2.0f );
			}

			ImGui::TreePop();
		}

		if ( ImGui::TreeNode( "Screen Shake Test" ) )
		{
			static float power = 20.0f;
			static float decel = 5.0f;
			static float time = 1.0f;
			static float interval = 0.05f;
			static Donya::ScreenShake::Kind kind = Donya::ScreenShake::Kind::MOMENT;

			ImGui::Text( "now X : %f\n", Donya::ScreenShake::GetX() );
			ImGui::Text( "now Y : %f\n", Donya::ScreenShake::GetY() );

			ImGui::SliderFloat( "Power", &power, 6.0f, 128.0f );
			ImGui::SliderFloat( "Deceleration", &decel, 0.2f, 64.0f );
			ImGui::SliderFloat( "ShakeTime", &time, 0.1f, 10.0f );
			ImGui::SliderFloat( "Interval", &interval, 0.1f, 3.0f );
			if ( ImGui::Button( "Toggle the kind" ) )
			{
				kind = ( kind == Donya::ScreenShake::MOMENT )
					? Donya::ScreenShake::PERMANENCE
					: Donya::ScreenShake::MOMENT;
			}
			ImGui::Text( "Now Kind : %s", ( kind == Donya::ScreenShake::MOMENT ) ? "Moment" : "Permanence" );

			if ( ImGui::Button( "Set Shake X" ) )
			{
				if ( Donya::Keyboard::Shifts() )
				{
					Donya::ScreenShake::SetX( kind, power );
				}
				else
				{
					Donya::ScreenShake::SetX( kind, power, decel, time, interval );
				}
			}
			if ( ImGui::Button( "Set Shake Y" ) )
			{
				if ( Donya::Keyboard::Shifts() )
				{
					Donya::ScreenShake::SetY( kind, power );
				}
				else
				{
					Donya::ScreenShake::SetY( kind, power, decel, time, interval );
				}
			}
			if ( ImGui::Button( "Stop Shake" ) )
			{
				Donya::ScreenShake::StopX();
				Donya::ScreenShake::StopY();
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

#endif // DEBUG_MODE && USE_IMGUI
}