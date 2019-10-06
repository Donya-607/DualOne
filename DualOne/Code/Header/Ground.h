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
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
//	bool			isEnable;
public:
	Tree();
//	Tree(Donya::Vector3 _pos);
	~Tree();
	void Init(Donya::Vector3 _pos);
	void Update();
	void Draw(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	bool ShouldErase(Donya::Vector3 _playerPos) const
	{
		return (pos.z  >= _playerPos.z + 1000) ? true : false;
	}

	void LoadModel();
	void Move();


	void UseImGui();

	// Getter
//	bool GetIsEnable() { return isEnable; }
	// Setter
//	void SetIsEnable(bool _result) { isEnable = _result; }
};

class Ground
{
	std::vector<Block>	block;
	std::vector<Tree>	trees;
//	std::array<Tree, 100> trees;
	int					timer;
public:
	Ground();
	~Ground();

	void CreateBlock();
	void CreateTree(Donya::Vector3 _pos);
	void Init();
	void Uninit();
//	void Update();
	void Update(Donya::Vector3 _playerPos);
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void EraseDeadTree(Donya::Vector3 _playerPos);
};