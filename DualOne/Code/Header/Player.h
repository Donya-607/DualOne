#pragma once

#include <memory>

#include "Donya/Collision.h"
#include "Donya/StaticMesh.h"
#include "Donya/Quaternion.h"
#include "Donya/Vector.h"		// Also include <DirectXMath>

class Player
{
private:
	int					currentLane;	// 0-based, count by left.
	int					nextLane;		// 0-based, count by left.
	AABB				hitBox;			// Local-space.
	Donya::Vector3		pos;			// World-space.
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;
	std::shared_ptr<Donya::StaticMesh>	pModel;
public:
	Player();
	~Player();
public:
	void Init();
	void Uninit();

	void Update();

	void Draw
	(
		const DirectX::XMFLOAT4X4	&matView,
		const DirectX::XMFLOAT4X4	&matProjection,
		const Donya::Vector4		&lightDirection,
		const Donya::Vector4		&cameraPos
	) const;
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
};
