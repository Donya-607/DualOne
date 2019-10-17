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

#if USE_IMGUI

void UIObject::CallSlidersOfImGui()
{
	ImGui::SliderFloat( u8"�p�x",				&degree,  0.0f, 360.0f );
	ImGui::SliderFloat( u8"�A���t�@",			&alpha,   0.0f, 1.0f );
	ImGui::SliderFloat2( u8"�ʒu�i�w�C�x�j",		&ssPos.x, 0.0f, 1920.0f );
	ImGui::SliderFloat2( u8"�X�P�[���i�w�C�x�j",	&scale.x, 0.0f, 1.0f );
}

#endif // USE_IMGUI
