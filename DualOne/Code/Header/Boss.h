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

// HACK:These object class has method is very similar, so I can put-together by Inheritance.

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
		Expose,
		Wait,
		Fly
	};
private:
	State				status;

	int					aliveFrame;
	int					waitFrame;
	
	float				exposingLength;	// Use when generated ~ wait.

	AABB				hitBox;			// The position is local-space, size is world-space.
	Donya::Vector3		pos;
	Donya::Vector3		basePos;
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
			archive( CEREAL_NVP( exposingLength ) );
		}
		if ( 3 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
public:
	void Init( const Donya::Vector3 &wsAppearPos );
	void Uninit();

	void Update( float basePositionZ );

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
	void ExposeUpdate();
	void WaitUpdate();
	void FlyUpdate();

	void Move();
};

CEREAL_CLASS_VERSION( Missile, 2 )

class Obstacle
{
public:
	class Warning
	{
	private:
		static size_t sprWarning;
		static Warning parameter;
	public:
		static void RegisterLaneCount( size_t newLaneCount );

		static void LoadSprite();

		static void LoadParameter( bool isBinary = true );

	#if USE_IMGUI

		static void SaveParameter();

		static void UseImGui();

	#endif // USE_IMGUI
	private:
		int showFrame;
		int laneNo;									// 0-based, count by left.
		std::vector<Donya::Vector2> ssPositions;	// Screen-space.
	public:
		Warning();
		~Warning();
	private:
		friend class cereal::access;
		template<class Archive>
		void serialize( Archive &archive, std::uint32_t version )
		{
			archive( CEREAL_NVP( showFrame ) );

			if ( 1 <= version )
			{
				archive( CEREAL_NVP( ssPositions ) );
			}
			if ( 2 <= version )
			{
				// archive( CEREAL_NVP( x ) );
			}
		}
		static constexpr const char *SERIAL_ID = "ObstacleWarning";
	public:
		void Init( int laneNo );

		void Update();

		void Draw() const;
	};
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
	Warning				warning;
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
	void Init( int laneNumber, const Donya::Vector3 &wsAppearPos );
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
CEREAL_CLASS_VERSION( Obstacle::Warning, 1 )

class Beam
{
private:
	static Beam parameter;
	static constexpr const char *SERIAL_ID = "Beam";
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
		Swing,
		Wait,
		End
	};
private:
	State				status;

	int					afterWaitFrame;		// Use after finish the rotate of angle.
	float				angleIncreaseSpeed;	// Use for beam-angle(radian).
	float				easeParam;			// Use for "t" of easing.
	float				beamAngle;			// Radian. 0 is right. rotate by CCW. use at YZ-plane.
	float				beamAngleBegin;		// Radian. 0 is right. rotate by CCW. use at YZ-plane.
	float				beamAngleEnd;		// Radian. 0 is right. rotate by CCW. use at YZ-plane.
	float				beamLength;
	AABB				hitBox;				// The position is local-space, size is world-space.
	Donya::Vector3		basePos;
	Donya::Vector3		beamDestPos;
public:
	Beam();
	~Beam();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( afterWaitFrame ),
			CEREAL_NVP( angleIncreaseSpeed ),
			CEREAL_NVP( beamAngleBegin ),
			CEREAL_NVP( beamAngleEnd ),
			CEREAL_NVP( beamLength ),
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

	void Update( float wsBasePositionZ );

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
	/// Returns hit-box is in world-space.
	/// </summary>
	AABB GetHitBox() const;

	bool ShouldErase() const;
private:
	void AngleUpdate();
	void WaitUpdate();

	void CalcBeamDestination();
};

CEREAL_CLASS_VERSION( Beam, 0 )

class Wave
{
private:
	static Wave parameter;
	// static std::shared_ptr<Donya::StaticMesh>	pModel;
	static constexpr const char *SERIAL_ID = "Wave";
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
	float				speed;		// Add to z-position.
	AABB				hitBox;		// The position is local-space, size is world-space.
	Donya::Vector3		pos;
	Donya::Quaternion	posture;
public:
	Wave();
	~Wave();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( aliveFrame ),
			CEREAL_NVP( speed ),
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
};

CEREAL_CLASS_VERSION( Wave, 0 )

class AttackParam : public Donya::Singleton<AttackParam>
{
	friend Donya::Singleton<AttackParam>;
public:
	enum AttackKind
	{
		Missile = 0,
		Obstacle,
		Beam,
		Wave,

		ATTACK_KIND_COUNT
	};
public:
	int maxHP;				// 1-based.
	int counterMax;
	int untilAttackFrame;	// Frame of begin the attack.
	int resetWaitFrame;		// Use when reset the count.
	int damageWaitFrame;	// Use when the boss receive a damage.
	int stunFrame;
	// int reuseFrame;
	std::vector<std::array<int, ATTACK_KIND_COUNT>> intervalsPerHP;		// Each attacks per HP(0-based).
	std::vector<std::array<int, ATTACK_KIND_COUNT>> reuseFramesPerHP;	// Each attacks per HP(0-based).
	std::vector<std::vector<int>> obstaclePatterns;					// Array of pattern, store 0(FALSE) or 1(TRUE). e.g. [0:T,F,F], [1:F,T,F], ...
private:
	AttackParam();
public:
	~AttackParam();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		if ( 2 <= version )
		{
			archive
			(
				CEREAL_NVP( maxHP ),
				CEREAL_NVP( counterMax ),
				CEREAL_NVP( untilAttackFrame ),
				CEREAL_NVP( resetWaitFrame ),
				CEREAL_NVP( intervalsPerHP ),
				CEREAL_NVP( reuseFramesPerHP ),
				CEREAL_NVP( obstaclePatterns )
			);

			if ( 3 <= version )
			{
				archive
				(
					CEREAL_NVP( damageWaitFrame ),
					CEREAL_NVP( stunFrame )
				);
			}
			if ( 4 <= version )
			{
				// archive( CEREAL_NVP( x ) );
			}

			return;
		}
		// else

		archive
		(
			CEREAL_NVP( counterMax ),
			// CEREAL_NVP( intervals ),
			CEREAL_NVP( obstaclePatterns )
		);

		if ( 1 <= version )
		{
			archive
			(
				CEREAL_NVP( untilAttackFrame )
				// CEREAL_NVP( reuseFrame )
			);
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

CEREAL_CLASS_VERSION( AttackParam, 3 )

class CollisionDetail : public Donya::Singleton<CollisionDetail>
{
	friend Donya::Singleton<CollisionDetail>;
private:
	static constexpr int LOWER_LEVEL_COUNT = 2;
public:
	int					levelCount;		// 1-based.
	std::vector<float>	levelBorders;	// [0:lv.1][1:lv.2]..., e.g.[ if ( [1] <= rhs ) { level = 2; }]
private:
	CollisionDetail();
public:
	~CollisionDetail();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( levelCount ),
			CEREAL_NVP( levelBorders )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "BossCollisionDetail";
public:
	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( CollisionDetail, 0 )

class Boss
{
private:
	enum class State
	{
		Normal,
		Stun,
	};
private:
	int									currentHP;		// 1-based, 0 express the dead.
	int									attackTimer;
	int									waitReuseFrame;	// Wait frame of until can reuse.
	int									stunTimer;

	float								maxDistanceToTarget;

	State								status;

	AABB								hitBox;

	Donya::Vector3						pos;
	Donya::Vector3						velocity;
	Donya::Vector3						stunVelocity;	// Use when stun.
	Donya::Vector3						missileOffset;	// The offset of appear position of missile. the x used to [positive:outer side][negative:inner side].
	Donya::Vector3						obstacleOffset;	// The offset of appear position of obstacle. the x used to [positive:outer side][negative:inner side].
	Donya::Vector3						beamOffset;		// The offset of appear position of beam. the x used to [positive:outer side][negative:inner side].
	Donya::Vector3						waveOffset;		// The offset of appear position of wave.
	Donya::Quaternion					posture;

	std::shared_ptr<Donya::StaticMesh>	pModelBody;
	std::shared_ptr<Donya::StaticMesh>	pModelFoot;
	std::shared_ptr<Donya::StaticMesh>	pModelRoll;

	std::vector<Donya::Vector3>			lanePositions;	// This value only change by initialize method.
	std::vector<Missile>				missiles;
	std::vector<Obstacle>				obstacles;
	std::vector<Beam>					beams;
	std::vector<Wave>					waves;
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
			archive( CEREAL_NVP( beamOffset ) );
		}
		if ( 5 <= version )
		{
			archive( CEREAL_NVP( waveOffset ) );
		}
		if ( 6 <= version )
		{
			archive( CEREAL_NVP( stunVelocity ) );
		}
		if ( 7 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Boss";
public:
	void Init( float initDistanceFromOrigin, const std::vector<Donya::Vector3> &registerLanePositions );
	void Uninit();

	void Update( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos );

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
	/// 1~ is alive. 0 is dead.
	/// </summary>
	/// <returns></returns>
	int GetCurrentHP() const { return currentHP; }
	/// <summary>
	/// Retruns position is in world-space.
	/// </summary>
	Donya::Vector3 GetPos() const { return pos; }
	/// <summary>
	/// Returns hit-box is in world-space.
	/// </summary>
	AABB GetHitBox() const;
	/// <summary>
	/// if ( GetCurrentHP() == 0 )
	/// </summary>
	bool IsDead() const;

	/// <summary>
	/// Please call Missile::HitToOther() when hit detected.
	/// </summary>
	const std::vector<Missile> &FetchReflectableMissiles() const;
	/// <summary>
	/// The obstacle can not reflection.<para></para>
	/// Please call Obstacle::HitToOther() when hit detected.
	/// </summary>
	const std::vector<Obstacle> &FetchObstacles() const;
	/// <summary>
	/// Returns hit-boxes is can not reflection.
	/// </summary>
	std::vector<AABB> FetchHitBoxes() const;

	void ReceiveImpact( Donya::Vector3 wsCollidedPosition );
private:
	void LoadModel();

	void Move( const Donya::Vector3 &wsAttackTargetPos );

	void LotteryAttack( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos );

	Donya::Vector3 LotteryLanePosition();
	
	void ShootMissile( int targetLaneNo, const Donya::Vector3 &wsAttackTargetPos );
	void UpdateMissiles();
	
	void GenerateObstacles( const Donya::Vector3 &wsAttackTargetPos );
	void UpdateObstacles();
	
	void ShootBeam();
	void UpdateBeams();
	
	void GenerateWave();
	void UpdateWaves();

	void UpdateAttacks();

	void ReceiveDamage( int damage );

	void StunUpdate();
	bool IsStunning() const;

	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( Boss, 6 )
