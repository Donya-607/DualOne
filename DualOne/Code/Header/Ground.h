#pragma once

#include <vector>
#include <array>
#include "Donya/Vector.h"
#include "Donya/GeometricPrimitive.h"
#include "Donya/StaticMesh.h"
#include "Donya/UseImgui.h"


class Block
{
private:
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
	Donya::Geometric::Cube cube;
	std::unique_ptr<Donya::Geometric::TextureBoard> billBoard;
public:
	Block();
	~Block();

	void Init(size_t _num);
	void Update(Donya::Vector3 _playerPos);
	void Draw
	(
		const DirectX::XMFLOAT4X4& matView,
		const DirectX::XMFLOAT4X4& matProjection,
		const DirectX::XMFLOAT4& lightDirection,
		const DirectX::XMFLOAT4& cameraPosition,
		bool isEnableFill = true
	);

	void Move();
	void ApplyLoopToMap(Donya::Vector3 _playerPos);
	Donya::Vector3 GetPos() { return pos; }
};

struct Tree
{
public:
	static std::shared_ptr<Donya::StaticMesh> pModel;
private:
	Donya::Vector3	pos;
	Donya::Vector3	velocity;
	Donya::Vector3	scale;
	Donya::Vector3	angle;
public:
	Tree();
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
		return (pos.z  >= _playerPos.z + 2000) ? true : false;
	}

	void LoadModel();

	// Getter
//	bool GetIsEnable() { return isEnable; }
	// Setter
//	void SetIsEnable(bool _result) { isEnable = _result; }
};

class Ground
{
	std::array<Block, 4> blocks;
	std::vector<Tree>	trees;
	int					timer;
public:
	static Donya::Vector3 treePos;
	static Donya::Vector3 treeAngle;
	static int CREATE_TREE;

public:
	Ground();
	~Ground();

	void CreateTree(Donya::Vector3 _pos);
	void Init();
	void Uninit();
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

#ifdef USE_IMGUI
	void UseImGui();
#endif

};