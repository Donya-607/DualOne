#pragma once

#include <memory>
#include <vector>

#include "Donya/Collision.h"
#include "Donya/StaticMesh.h"
#include "Donya/Quaternion.h"
#include "Donya/UseImgui.h"		// Use USE_IMGUI macro.
#include "Donya/Vector.h"		// Also include <DirectXMath>

class ReflectedEntity
{
private:
	static std::shared_ptr<Donya::StaticMesh> pModel;
	/// <summary>
	/// Load model if has not loaded.
	/// </summary>
public:
	static void LoadModel();
private:
	float				gravity;
	Sphere				hitBox;
	Donya::Vector3		wsPos;
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;
public:
	ReflectedEntity();
	~ReflectedEntity();
public:
	void Init( float gravity, Sphere hitBox, Donya::Vector3 wsPos, Donya::Vector3 velocity );
	void Uninit();

	void Update();

	void Draw
	(
		const DirectX::XMFLOAT4X4 &matView,
		const DirectX::XMFLOAT4X4 &matProjection,
		const Donya::Vector4 &lightDirection,
		const Donya::Vector4 &cameraPos
	) const;
public:
	bool ShouldErase() const;
	/// <summary>
	/// Returns position is belong world-space.
	/// </summary>
	Donya::Vector3 GetPos() const { return wsPos; };
	/// <summary>
	/// Returns hit-box is belong world-space.
	/// </summary>
	Sphere GetHitBox() const;
};

class Player
{
public:
	struct Input
	{
		Donya::Vector2 stick; // Normalized.
		bool doCharge;
	public:
		Input() :
			stick(),
			doCharge( false )
		{}
		Input( Donya::Vector2 stick, bool doCharge ) :
			stick( stick ),
			doCharge( doCharge )
		{}
	};
private:
	enum class State
	{
		Run,	// Normal status.
		Charge,	// 
		Jump,	// 
		Stun	// I can't move.
	};
private:
	State				status;
	int					currentLane;	// 0-based, count by left.
	int					laneCount;		// 0-based, count by left.
	int					stunTimer;
	float				charge;			// 0.0f ~ 1.0f.
	AABB				hitBox;			// Local-space.
	Donya::Vector3		pos;			// World-space.
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;
	std::shared_ptr<Donya::StaticMesh>	pModel;
	std::vector<Donya::Vector3>			lanePositions;	// This value only change by initialize method.
public:
	Player();
	~Player();
public:
	void Init( const std::vector<Donya::Vector3> &lanePositions );
	void Uninit();

	void Update( Input input );

	void Draw
	(
		const DirectX::XMFLOAT4X4	&matView,
		const DirectX::XMFLOAT4X4	&matProjection,
		const Donya::Vector4		&lightDirection,
		const Donya::Vector4		&cameraPos
	) const;
public:
	struct CollideResult
	{
		float			gravity{};
		Sphere			hitBox{};
		Donya::Vector3	wsPos{};
		Donya::Vector3	velocity{};
		bool shouldGenerateBullet{};
	};
	CollideResult ReceiveImpact( bool canReflection );
public:
	int GetCurrentLane() const { return currentLane; }

	/// <summary>
	/// Returns hit-box belong world-space.
	/// </summary>
	AABB GetHitBox() const;
	/// <summary>
	/// Returns position belong world-space.
	/// </summary>
	Donya::Vector3 GetPos() const { return pos; }
private:
	void LoadModel();
	void ApplyExternalParameter();

	void ChooseCurrentStateUpdate( Input input );

	void RunInit();
	/// <summary>
	/// Run if not stunning.
	/// </summary>
	void RunUpdate( Input input );

	void ChargeInit();
	void ChargeUpdate( Input input );
	
	bool IsCharging() const;
	bool IsFullCharged() const;

	void ChangeLaneIfRequired( Input input );
	/// <summary>
	/// Move if when changing lane. called by RunUpdate() and ChargeUpdate().
	/// </summary>
	void HorizontalMove();
	/// <summary>
	/// Judge to close(true) when the distance to current-lane is lower-equal than little-change-lane-speed.
	/// </summary>
	bool IsCloseToCurrentLane( const Donya::Vector3 &wsJudgePosition ) const;
	bool IsChangingLane() const;

	float CalcGravity();
	void JumpInit();
	void JumpUpdate( Input input );
	bool IsJumping() const;

	void Landing();

	/// <summary>
	/// Add "velocity" to "pos", the "pos" is only changed by this method.
	/// </summary>
	void ApplyVelocity();

	void MakeStun();
	void StunUpdate( Input input );

#if USE_IMGUI

	void ShowParamToImGui() const;

#endif // USE_IMGUI
};
