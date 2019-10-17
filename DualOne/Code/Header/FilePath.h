#pragma once

#include <string>

#include "Donya/Serializer.h"

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension );

enum class SpriteAttribute
{
	FMODLogoBlack,
	FMODLogoWhite,
	Font,
	TitleLogo,
	Usage,
	Sentences,
	Warning,
	GroundTex1,
	GroundTex2,
	SledEffect,
	SmokeEffect,
	SmokeWhiteEffect,
};
std::wstring GetSpritePath( SpriteAttribute fileAttribute );

enum class ModelAttribute
{
	Box,
	Player,
	ReflectedEntity,
	BossArm,
	BossBody,
	BossFoot,
	BossRoll,
	Missile,
	Tree,
	Obstacle,
	Wave,
	Beam,
	SideWall,
};
std::string GetModelPath( ModelAttribute fileAttribute );
