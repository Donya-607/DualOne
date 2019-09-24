#pragma once

#include <string>

#include "Serializer.h"

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension );

int CalcStageCount();

std::string GetTerrainPath( int stageNumber );
std::string GetEnemyPath( int stageNumber );
std::string GetObjectPath( int stageNumber );
std::string GetConfigPath( int stageNumber );

enum class SpriteAttribute
{
	FMODLogoBlack,
	FMODLogoWhite,
	Player,
	PlayerEffects,
	MapChip,
	Enemy,
	Object,
	TitleLogo,
	SelectableStage,
	Usage,
	BG_Title,
	BG_1,
	BG_2,
	BG_3,
	UI,
	ScoreBoard,
};

std::wstring GetSpritePath( SpriteAttribute fileAttribute );
