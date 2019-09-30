#pragma once

#include <string>

#include "Donya/Serializer.h"

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension );

enum class SpriteAttribute
{
	FMODLogoBlack,
	FMODLogoWhite,
	TestFont,
};
std::wstring GetSpritePath( SpriteAttribute fileAttribute );

enum class ModelAttribute
{
	Box,
	Player,
};
std::string GetModelPath( ModelAttribute fileAttribute );
