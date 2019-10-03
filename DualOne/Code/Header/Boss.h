#pragma once

#include "Donya/Vector.h"
#include "Donya/Useful.h"
#include "Donya/GeometricPrimitive.h"

class Missle
{
	enum Status
	{
		NOT_ENABLE,
		INITIALIZE,
		PREP_MOVE,
		PREP_STOP,
		ATTACK_MOVE,
		END,
	};

	Donya::Geometric::Sphere sphere;

	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
	Status			state;
	bool		isShotFromRight;

public:
	void Init();
	void Uninit();
	void Update(Donya::Vector3 bossPos);
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void Move(Donya::Vector3 bossPos);


	/*------------------------------*/
	//	Getter and Setter
	/*------------------------------*/
	Donya::Vector3 Getpos() { return pos; }
};

class Boss
{
	Donya::Geometric::Cube cube;
	std::vector<Missle> missle;

	Donya::Vector3 pos;
	Donya::Vector3 velocity;
	Donya::Vector3 scale;
public:
	void Init();
	void Uninit();
	void Update();
	void Draw
	(	
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

private:
	void Move();
};