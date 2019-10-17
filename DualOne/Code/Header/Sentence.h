#pragma once

#include <vector>

#include "cereal/types/vector.hpp"

#include "Donya/Constant.h"
#include "Donya/Serializer.h"
#include "Donya/Template.h"
#include "Donya/UseImgui.h"

#include "Donya/Vector.h"

class Sentence final : public Donya::Singleton<Sentence>
{
	friend class Donya::Singleton<Sentence>;
public:
	enum class Kind
	{
		Pause,
		Retry,
		NewRecord,
		ClearTime,
		BestTime,
		BackToGame,
		BackToTitle,
		HoldingButton_,
		ReleaseThenStart,
		GAME_CLEAR,
		GAME_OVER,
		LeftArrow,
		RightArrow,
		ButtonA,
		CrossButton,
		Stick,

		KIND_COUNT
	};
	static constexpr int KIND_COUNT = scast<int>( Kind::KIND_COUNT );
private:
	size_t sprite;
	std::vector<Donya::Vector2> texPartPos;
	std::vector<Donya::Vector2> texPartSize;
private:
	Sentence();
public:
	~Sentence();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( texPartPos ),
			CEREAL_NVP( texPartSize )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
	static constexpr const char *SERIAL_ID = "Sentences";
public:
	void LoadSprite();

	void Draw( Kind kind, Donya::Vector2 ssPos, float degree = 0.0f, float alpha = 1.0f, float scale = 1.0f ) const;
public:
	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( Sentence, 0 )
