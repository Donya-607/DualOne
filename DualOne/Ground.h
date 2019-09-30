#pragma once

#include <vector>
#include <array>
#include "Donya/Vector.h"
#include "Donya/GeometricPrimitive.h"

class Block
{
private:
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	size;
	std::array<Donya::Geometric::Cube, 7> cube;

public:
	Block();
	~Block();

	void Init();
	void Update();
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void Move();
};

class Ground
{
	std::vector<Block>	block;
	int					timer;
public:
	Ground();
	~Ground();

	void Create();
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
};