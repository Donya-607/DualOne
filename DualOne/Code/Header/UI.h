#pragma once

#include "Donya/Serializer.h"
#include "Donya/Vector.h"
#include "Donya/UseImgui.h"

struct UIObject
{
public:
	size_t sprite{};
	float  degree{};
	float  alpha{};
	Donya::Vector2 ssPos{};	// screen-space.
	Donya::Vector2 scale{};
public:
	UIObject();
	virtual ~UIObject();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( degree ),
			CEREAL_NVP( alpha ),
			CEREAL_NVP( ssPos ),
			CEREAL_NVP( scale )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP( x ) );
		}
	}
public:
	void LoadSprite( const std::wstring &filePath, size_t maxInstanceCount );

	void Draw();

#if USE_IMGUI

public:

	// You must call at between ImGui::Begin() and ImGui::End().
	void CallSlidersOfImGui();

#endif // USE_IMGUI
};

CEREAL_CLASS_VERSION( UIObject, 0 )
