#pragma once

#include <string>

#include "Donya/Serializer.h"

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension );

enum class SpriteAttribute
{
	FMODLogoBlack,
	FMODLogoWhite,
	TestFont,
	GroundTex1,
	GroundTex2,
	SledEffect,
};
std::wstring GetSpritePath( SpriteAttribute fileAttribute );

enum class ModelAttribute
{
	Box,
	Player,
	ReflectedEntity,
	BossBody,
	BossFoot,
	BossRoll,
	Missile,
	Tree,
	Obstacle,
};
std::string GetModelPath( ModelAttribute fileAttribute );
