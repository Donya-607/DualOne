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
	int					aliveFrame;
	int					waitFrame;

	AABB				hitBox;		// The position is local-space, size is world-space.
	Donya::Vector3		pos;
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;

	mutable bool		wasHitToOther;
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

	bool ShouldErase() const;

	/// <summary>
	/// Please call when hit to anything.
	/// </summary>
	void HitToOther() const;
private:
	void Move();
};

CEREAL_CLASS_VERSION( Missile, 1 )

class Obstacle
{
private:
	static Obstacle parameter;
	static std::shared_ptr<Donya::StaticMesh>	pModel;
	static constexpr const char *SERIAL_ID = "Obstacle";
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
	float				decelSpeed;	// Add to z-position. this works like deceleration.
	AABB				hitBox;		// The position is local-space, size is world-space.
	Donya::Vector3		pos;
	Donya::Quaternion	posture;

	mutable bool		wasHitToOther;
public:
	Obstacle();
	~Obstacle();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( decelSpeed ),
			CEREAL_NVP( hitBox )
		);
		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
public:
	void Init( const Donya::Vector3 &wsAppearPos );
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

	bool ShouldErase() const;

	/// <summary>
	/// Please call when hit to anything.
	/// </summary>
	void HitToOther() const;
private:
};

CEREAL_CLASS_VERSION( Obstacle, 0 )

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

	float								maxDistanceToTarget;

	AABB								hitBox;

	Donya::Vector3						pos;
	Donya::Vector3						velocity;
	Donya::Vector3						missileOffset;	// The offset of appear position of missile. the x used to [positive:outer side][negative:inner side].
	Donya::Vector3						obstacleOffset;	// The offset of appear position of obstacle. the x used to [positive:outer side][negative:inner side].
	Donya::Quaternion					posture;

	std::shared_ptr<Donya::StaticMesh>	pModel;

	std::vector<Donya::Vector3>			lanePositions;	// This value only change by initialize method.
	std::vector<Missile>				missiles;
	std::vector<Obstacle>				obstacles;
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
			archive( CEREAL_NVP( maxDistanceToTarget ) );
		}
		if ( 3 <= version )
		{
			archive( CEREAL_NVP( obstacleOffset ) );
		}
		if ( 4 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Boss";
public:
	void Init( float initDistanceFromOrigin, const std::vector<Donya::Vector3> &registerLanePositions );
	void Uninit();

	void Update( const Donya::Vector3 &wsAttackTargetPos );

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

	/// <summary>
	/// Please call Missile::HitToOther() when hit detected.
	/// </summary>
	const std::vector<Missile> &FetchReflectableMissiles() const;
	/// <summary>
	/// The obstacle can not reflection.<para></para>
	/// Please call Obstacle::HitToOther() when hit detected.
	/// </summary>
	const std::vector<Obstacle> &FetchObstacles() const;
private:
	void LoadModel();

	void Move( const Donya::Vector3 &wsAttackTargetPos );

	void LotteryAttack( const Donya::Vector3 &wsAttackTargetPos );
	Donya::Vector3 LotteryLanePosition();

	void ShootMissile( const Donya::Vector3 &wsAttackTargetPos );
	void UpdateMissiles();
	
	void GenerateObstacles( const Donya::Vector3 &wsAttackTargetPos );
	void UpdateObstacles();

	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( Boss, 3 )
