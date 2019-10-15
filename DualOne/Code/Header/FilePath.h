#pragma once

#include <string>

#include "Donya/Serializer.h"

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension );

enum class SpriteAttribute
{
	FMODLogoBlack,
	FMODLogoWhite,
	TestFont,
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
};
std::string GetModelPath( ModelAttribute fileAttribute );
