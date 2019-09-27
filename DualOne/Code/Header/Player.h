#pragma once

#include "Donya/Quaternion.h"
#include "Donya/Vector.h"

class Player
{
private:
	int					currentLane;	// 0-based, count by left.
	int					nextLane;		// 0-based, count by left.
	Donya::Vector3		pos;			// World-space.
	Donya::Vector3		velocity;
	Donya::Quaternion	posture;
public:
	Player();
	~Player();
public:
	void Init();
	void Uninit();

	void Update();

	void Draw() const;
public:
	int GetCurrentLane() const { return currentLane; }

	Donya::Vector3 GetWorldPos() const { return pos; }
};
