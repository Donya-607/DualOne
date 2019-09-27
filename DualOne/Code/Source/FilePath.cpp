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

std::sstring GetModelPath( ModelAttribute fileAttribute )
{
	switch ( fileAttribute )
	{
	case ModelAttribute::Box:
		return "./Data/Model/Box/Box.bin";			// break;
	case ModelAttribute::Player:
		return "./Data/Model/Player/Player.bin";	// break;
	default:
		assert( !"Error : Specified unexpect model type." ); break;
	}

	return "ERROR_ATTRIBUTE";
}
