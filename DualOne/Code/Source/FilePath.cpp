#include "FilePath.h"

#include "Constant.h"	// Use DEBUG_MODE.
#include "Useful.h"		// Use IsExistFile().

std::string GenerateSerializePath( std::string identifier, Serializer::Extension extension )
{
	std::string ext{};
	switch ( extension )
	{
	case Serializer::Extension::BINARY:	ext = ".bin";	break;
	case Serializer::Extension::JSON:	ext = ".json";	break;
	default: return "ERROR_EXTENSION";
	}

	return "./Data/Parameters/" + identifier + ext;
}

int CalcStageCount()
{
	constexpr int INITIAL_NO = -1;

	static int stageCount = INITIAL_NO;
	if ( stageCount != INITIAL_NO ) { return stageCount; }
	// else

	std::string directory{ "./Data/Stages/" };
	std::string fileName{ "_Terrain.txt" };
	std::string findName{};

	for ( int i = 0; true; ++i )
	{
		findName = directory + std::to_string( i ) + fileName;

		if ( !Donya::IsExistFile( findName ) )
		{
			stageCount = i;
			break;
		}
	}

	return stageCount;
}

std::string GetTerrainPath( int stageNo )
{
	std::string directory{ "./Data/Stages/" };

	std::string fileName = std::to_string( stageNo ) + "_Terrain.txt";

	return directory + fileName;
}
std::string GetEnemyPath( int stageNo )
{
	std::string directory{ "./Data/Stages/" };

	std::string fileName = std::to_string( stageNo ) + "_Enemy.txt";

	return directory + fileName;
}
std::string GetObjectPath( int stageNo )
{
	std::string directory{ "./Data/Stages/" };

	std::string fileName = std::to_string( stageNo ) + "_Object.txt";

	return directory + fileName;
}
std::string GetConfigPath( int stageNo )
{
	std::string directory{ "./Data/Stages/" };

	std::string fileName = std::to_string( stageNo ) + "_Config.txt";

	return directory + fileName;
}

std::wstring GetSpritePath( SpriteAttribute fileAttribute )
{
	switch ( fileAttribute )
	{
	case SpriteAttribute::FMODLogoBlack:
		return L"./Data/Images/Rights/FMOD Logo Black - White Background.png";	// break;
	case SpriteAttribute::FMODLogoWhite:
		return L"./Data/Images/Rights/FMOD Logo White - Black Background.png";	// break;
	case SpriteAttribute::Player:
		return L"./Data/Images/Player/Player.png";	// break;
	case SpriteAttribute::PlayerEffects:
		return L"./Data/Images/Player/Effect.png";	// break;
	case SpriteAttribute::MapChip:
		return L"./Data/Images/BG/MapChip.png";		// break;
	case SpriteAttribute::Enemy:
		return L"./Data/Images/Enemy/Enemy.png";	// break;
	case SpriteAttribute::Object:
		return L"./Data/Images/Object/Object.png";	// break;
	case SpriteAttribute::TitleLogo:
		return L"./Data/Images/Title/Logo.png";		// break;
	case SpriteAttribute::SelectableStage:
		return L"./Data/Images/Title/Stages.png";	// break;
	case SpriteAttribute::Usage:
		return L"./Data/Images/UI/usage.png";		// break;
	case SpriteAttribute::BG_Title:
		return L"./Data/Images/BG/Title.png";		// break;
	case SpriteAttribute::BG_1:
		return L"./Data/Images/BG/BG_1.png";		// break;
	case SpriteAttribute::BG_2:
		return L"./Data/Images/BG/BG_2.png";		// break;
	case SpriteAttribute::BG_3:
		return L"./Data/Images/BG/BG_3.png";		// break;
	case SpriteAttribute::UI:
		return L"./Data/Images/UI/Statements.png";	// break;
	case SpriteAttribute::ScoreBoard:
		return L"./Data/Images/UI/ScoreBoard.png";	// break;
	default:
		assert( !"Error : Specified unexpect sprite type." ); break;
	}

	return L"ERROR_ATTRIBUTE";
}
