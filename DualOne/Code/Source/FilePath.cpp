#include "FilePath.h"

#include "Donya/Constant.h"		// Use DEBUG_MODE.
#include "Donya/Useful.h"		// Use IsExistFile().

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

std::wstring GetSpritePath( SpriteAttribute fileAttribute )
{
	switch ( fileAttribute )
	{
	case SpriteAttribute::FMODLogoBlack:
		return L"./Data/Images/Rights/FMOD Logo Black - White Background.png";	// break;
	case SpriteAttribute::FMODLogoWhite:
		return L"./Data/Images/Rights/FMOD Logo White - Black Background.png";	// break;
	case SpriteAttribute::TestFont:
		return L"./Data/Images/Font/TestFont.png";	// break;
	default:
		assert( !"Error : Specified unexpect sprite type." ); break;
	}

	return L"ERROR_ATTRIBUTE";
}

std::string GetModelPath( ModelAttribute fileAttribute )
{
	switch ( fileAttribute )
	{
	case ModelAttribute::Box:
		return "./Data/Models/Box/Box.bin";					// break;
	case ModelAttribute::Player:
		return "./Data/Models/Player/Player.bin";			// break;
	case ModelAttribute::ReflectedEntity:
		return "./Data/Models/Player/ReflectedEntity.bin";	// break;
	case ModelAttribute::Boss:
		return "./Data/Models/Boss/Boss.bin";				// break;
	case ModelAttribute::Missile:
		return "./Data/Models/Boss/Missile.bin";			// break;
	default:
		assert( !"Error : Specified unexpect model type." ); break;
	}

	return "ERROR_ATTRIBUTE";
}
