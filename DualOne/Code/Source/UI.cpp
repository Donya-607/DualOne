#include "UI.h"

#include "Donya/Sprite.h"

UIObject::UIObject() :
	sprite( NULL ),
	degree(), alpha( 1.0f ),
	ssPos(), scale( 1.0f, 1.0f )
{

}
UIObject::~UIObject() = default;

void UIObject::LoadSprite( const std::wstring &filePath, size_t maxCount )
{
	sprite = Donya::Sprite::Load( filePath, maxCount );
}

void UIObject::Draw()
{
	Donya::Sprite::DrawExt
	(
		sprite,
		ssPos.x, ssPos.y,
		scale.x, scale.y,
		degree, alpha
	);
}
