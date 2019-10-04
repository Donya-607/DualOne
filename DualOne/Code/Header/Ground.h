#pragma once

#include <vector>
#include <array>
#include "Donya/Vector.h"
#include "Donya/GeometricPrimitive.h"
#include "Donya/StaticMesh.h"

class Block
{
private:
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
	Donya::Geometric::Cube cube;

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

struct Tree
{
public:
	static std::shared_ptr<Donya::StaticMesh> pModel;
private:
	Donya::Vector3 pos;
	Donya::Vector3 velocity;
	Donya::Vector3 scale;
public:
	Tree();
	Tree(Donya::Vector3 _pos);
	~Tree();
	void Update();
	void Draw(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void LoadModel();
	void Move();

	void UseImGui();
};

class Ground
{
	std::vector<Block>	block;
	std::vector<Tree>	trees;
	int					timer;
public:
	Ground();
	~Ground();

	void CreateBlock();
	void CreateTree(Donya::Vector3 _pos);
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