#pragma once

#include "Donya/Vector.h"
#include "Donya/Useful.h"
#include "Donya/GeometricPrimitive.h"

class Boss
{
	Donya::Geometric::Cube cube;
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