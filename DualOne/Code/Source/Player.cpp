#include "Player.h"

#include "Donya/Constant.h"
#include "Donya/Loader.h"
#include "Donya/Serializer.h"
#include "Donya/Template.h"
#include "Donya/Useful.h"
#include "Donya/UseImgui.h"

#include "Common.h"
#include "FilePath.h"

struct PlayerParameter final : public Donya::Singleton<PlayerParameter>
{
	friend class Donya::Singleton<PlayerParameter>;
public:
	AABB hitBox{}; // Store local-space.
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive( CEREAL_NVP( hitBox ) );

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Player";
public:

	void LoadParameter( bool isBinary = true )
	{
		Serializer::Extension ext = ( isBinary )
		? Serializer::Extension::BINARY
		: Serializer::Extension::JSON;
		std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

		Serializer seria;
		seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
	}

#if USE_IMGUI

	void SaveParameter()
	{
		Serializer::Extension bin = Serializer::Extension::BINARY;
		Serializer::Extension json = Serializer::Extension::JSON;
		std::string binPath = GenerateSerializePath( SERIAL_ID, bin );
		std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

		Serializer seria;
		seria.Save( bin, binPath.c_str(), SERIAL_ID, *this );
		seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
	}

	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( u8"プレイヤー" ) )
			{
				if ( ImGui::TreeNode( u8"当たり判定" ) )
				{
					auto EnumAABBParamToImGui = []( AABB *pAABB )
					{
						if ( !pAABB ) { return; }
						// else

						constexpr float RANGE = 64.0f;
						ImGui::SliderFloat3( u8"原点のオフセット（ＸＹＺ）",	&pAABB->pos.x,	-RANGE,	RANGE );
						ImGui::SliderFloat3( u8"サイズの半分（ＸＹＺ）",		&pAABB->size.x,	0.0f,	RANGE );
					};

					EnumAABBParamToImGui( &hitBox );

					ImGui::TreePop();
				}
				
				if ( ImGui::TreeNode( u8"ファイル" ) )
				{
					static bool isBinary = true;
					if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
					if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
					std::string loadStr{ "読み込み " };
					loadStr += ( isBinary ) ? "Binary" : "JSON";

					if ( ImGui::Button( u8"保存" ) )
					{
						SaveParameter();
					}
					if ( ImGui::Button( Donya::MultiToUTF8( loadStr ).c_str() ) )
					{
						LoadParameter( isBinary );
					}

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}

#endif // USE_IMGUI

};

CEREAL_CLASS_VERSION( PlayerParameter, 0 )

Player::Player() :
	currentLane(), nextLane(),
	hitBox(),
	pos(), velocity(),
	posture( 0.0f, 0.0f, 0.0f, 1.0f ),
	pModel( nullptr )
{

}
Player::~Player() = default;

void Player::Init()
{
	LoadModel();

	PlayerParameter::Get().LoadParameter();

	ApplyExternalParameter();

	posture = Donya::Quaternion::Make( 0.0f, ToRadian( 180.0f ), 0.0f );
}
void Player::Uninit()
{
	// No op.
}

#if DEBUG_MODE
#include "Donya/Keyboard.h"
#endif // DEBUG_MODE

void Player::Update()
{
#if USE_IMGUI

	PlayerParameter::Get().UseImGui();
	ApplyExternalParameter();

#endif // USE_IMGUI

#if DEBUG_MODE

	// Move for test.
	{
		constexpr float MOVE_SPEED = 0.1f;
		Donya::Vector3 velocity{};

		namespace Input = Donya::Keyboard;

		if ( Input::Press( VK_UP		) ) { velocity.y += MOVE_SPEED; }
		if ( Input::Press( VK_DOWN		) ) { velocity.y -= MOVE_SPEED; }
		if ( Input::Press( VK_LEFT		) ) { velocity.x -= MOVE_SPEED; }
		if ( Input::Press( VK_RIGHT		) ) { velocity.x += MOVE_SPEED; }
		if ( Input::Press( VK_RSHIFT	) ) { velocity.z -= MOVE_SPEED; }
		if ( Input::Press( VK_END		) ) { velocity.z += MOVE_SPEED; }

		pos += velocity;
	}

#endif // DEBUG_MODE

}

#if DEBUG_MODE
#include "Donya/GeometricPrimitive.h" // For drawing collision.
#endif // DEBUG_MODE
void Player::Draw( const DirectX::XMFLOAT4X4 &matView, const DirectX::XMFLOAT4X4 &matProjection, const Donya::Vector4 &lightDirection, const Donya::Vector4 &cameraPos ) const
{
	using namespace DirectX;

	auto Matrix		= []( const XMFLOAT4X4 &matrix )
	{
		return XMLoadFloat4x4( &matrix );
	};
	auto Float4x4	= []( const XMMATRIX &M )
	{
		XMFLOAT4X4 matrix{};
		XMStoreFloat4x4( &matrix, M );
		return matrix;
	};

	XMMATRIX S = XMMatrixIdentity();
	XMMATRIX R = Matrix( posture.RequireRotationMatrix() );
	XMMATRIX T = XMMatrixTranslation( pos.x, pos.y, pos.z );
	XMMATRIX W = S * R * T;

	XMMATRIX WVP = W * Matrix( matView ) * Matrix( matProjection );

	constexpr XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	pModel->Render
	(
		Float4x4( WVP ),
		Float4x4( W ),
		lightDirection,
		color,
		cameraPos
	);

#if DEBUG_MODE

	if ( Common::IsShowCollision() )
	{
		AABB wsBody = GetHitBox();

		XMMATRIX colS = XMMatrixScaling( wsBody.size.x, wsBody.size.y, wsBody.size.z );
		XMMATRIX colT = XMMatrixTranslation( wsBody.pos.x, wsBody.pos.y, wsBody.pos.z );
		XMMATRIX colW = colS * R * colT;

		XMMATRIX colWVP = colW * Matrix( matView ) * Matrix( matProjection );

		constexpr XMFLOAT4 colColor{ 0.0f, 0.9f, 0.5f, 0.4f };

		auto InitializedCube = []()
		{
			Donya::Geometric::Cube cube{};
			cube.Init();
			return cube;
		};
		static Donya::Geometric::Cube cube = InitializedCube();
		cube.Render
		(
			Float4x4( colWVP ),
			Float4x4( colW ),
			lightDirection,
			colColor
		);

	}

#endif // DEBUG_MODE
}

AABB Player::GetHitBox() const
{
	AABB wsHitBox = hitBox;
	wsHitBox.pos += pos;
	return wsHitBox;
}

void Player::LoadModel()
{
	Donya::Loader loader{};
	bool result = loader.Load( GetModelPath( ModelAttribute::Player ), nullptr );

	_ASSERT_EXPR( result, L"Failed : Load player model." );

	pModel = Donya::StaticMesh::Create( loader );

	_ASSERT_EXPR( pModel, L"Failed : Load player model." );
}

void Player::ApplyExternalParameter()
{
	hitBox = PlayerParameter::Get().hitBox;
}
