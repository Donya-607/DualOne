#pragma once

#include <array>
#include <memory>
#include <vector>

#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

#include "Donya/Collision.h"
#include "Donya/Quaternion.h"
#include "Donya/Serializer.h"
#include "Donya/StaticMesh.h"
#include "Donya/Template.h"
#include "Donya/UseImgui.h"
#include "Donya/Vector.h"

class Missile
{
private:
	static Missile parameter;
	static std::shared_ptr<Donya::StaticMesh>	pModel;
	static constexpr const char *SERIAL_ID = "Missile";
public:
	/// <summary>
	/// Load model if has not loaded.
	/// </summary>
	static void LoadModel();

	static void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	static void SaveParameter();

	static void UseImGui();

#endif // USE_IMGUI
private:
	enum class State
	{
		NOT_ENABLE,
		INITIALIZE,
		PREP_MOVE,
		PREP_STOP,
		ATTACK_MOVE,
		END,
	};
private:
	State				status;

	int					aliveFrame;
	int					waitFrame;

	AABB				hitBox;		// The position is local-space, size is world-space.
	Donya::Vector3		pos;
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;

	bool				isShotFromRight;
public:
	Missile();
	~Missile();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( waitFrame ),
			CEREAL_NVP( hitBox ),
			CEREAL_NVP( velocity )
		);
		if ( 1 <= version )
		{
			archive( CEREAL_NVP( aliveFrame ) );
		}
		if ( 2 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
public:
	void Init( const Donya::Vector3 &wsAppearPos );
	void Uninit();

	void Update( Donya::Vector3 bossPos );

	void Draw
	(
		const DirectX::XMFLOAT4X4 &matView,
		const DirectX::XMFLOAT4X4 &matProjection,
		const DirectX::XMFLOAT4 &lightDirection,
		const DirectX::XMFLOAT4 &cameraPosition,
		bool isEnableFill = true
	) const;
public:
	/// <summary>
	/// Retruns position is in world-space.
	/// </summary>
	Donya::Vector3 GetPos() const { return pos; }
	/// <summary>
	/// Returns hit-box is in world-space.
	/// </summary>
	AABB GetHitBox() const;

	bool ShouldErase() const;
private:
	void Move( Donya::Vector3 bossPos );
};

CEREAL_CLASS_VERSION( Missile, 1 )

class AttackParam : public Donya::Singleton<AttackParam>
{
	friend Donya::Singleton<AttackParam>;
public:
	enum AttackKind
	{
		Missile = 0,
		Obstacle,

		ATTACK_KIND_COUNT
	};
public:
	int counterMax;
	int untilAttackFrame;	// Frame of begin the attack.
	int reuseFrame;
	std::array<int, ATTACK_KIND_COUNT> intervals;
	std::vector<std::vector<int>> obstaclePatterns;	// Array of pattern, store 0(FALSE) or 1(TRUE). e.g. [0:T,F,F], [1:F,T,F], ...
private:
	AttackParam();
public:
	~AttackParam();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( counterMax ),
			CEREAL_NVP( intervals ),
			CEREAL_NVP( obstaclePatterns )
		);

		if ( 1 <= version )
		{
			archive
			(
				CEREAL_NVP( untilAttackFrame ),
				CEREAL_NVP( reuseFrame )
			);
		}
		if ( 2 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "BossAttackParameter";
public:
	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( AttackParam, 1 )

class Boss
{
	int									attackTimer;
	int									waitReuseFrame;	// Wait frame of until can reuse.

	AABB								hitBox;

	Donya::Vector3						pos;
	Donya::Vector3						velocity;
	Donya::Vector3						missileOffset;	// The offset of appear position of missile. the x used to [positive:outer side][negative:inner side].
	Donya::Quaternion					posture;

	std::shared_ptr<Donya::StaticMesh>	pModel;

	std::vector<Missile>				missiles;
	std::vector<Donya::Vector3>			lanePositions;	// This value only change by initialize method.
public:
	Boss();
	~Boss();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( hitBox ),
			CEREAL_NVP( velocity )
		);
		if ( 1 <= version )
		{
			archive( CEREAL_NVP( missileOffset ) );
		}
		if ( 2 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Boss";
public:
	void Init( float initDistanceFromOrigin, const std::vector<Donya::Vector3> &registerLanePositions );
	void Uninit();

	void Update();

	void Draw
	(
		const DirectX::XMFLOAT4X4 &matView,
		const DirectX::XMFLOAT4X4 &matProjection,
		const DirectX::XMFLOAT4 &lightDirection,
		const DirectX::XMFLOAT4 &cameraPosition,
		bool isEnableFill = true
	) const;
public:
	/// <summary>
	/// Retruns position is in world-space.
	/// </summary>
	Donya::Vector3 GetPos() const { return pos; }
	/// <summary>
	/// Returns hit-box is in world-space.
	/// </summary>
	AABB GetHitBox() const;
private:
	void ShootMissile();
private:
	void LoadModel();

	void Move();

	void LotteryAttack();

	void UpdateMissiles();

	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( Boss, 1 )
