#include "Sentence.h"

#include "Donya/Sprite.h"
#include "Donya/Useful.h"

#include "FilePath.h"

Sentence::Sentence() :
	sprite(),
	texPartPos(), texPartSize()
{
	texPartPos.resize( KIND_COUNT );
	texPartSize.resize( KIND_COUNT );
}
Sentence::~Sentence()
{
	texPartPos.clear();
	texPartPos.shrink_to_fit();
	texPartSize.clear();
	texPartSize.shrink_to_fit();
}

void Sentence::LoadSprite()
{
	if ( sprite != NULL ) { return; }
	// else

	sprite = Donya::Sprite::Load( GetSpritePath( SpriteAttribute::Sentences ), 128U );
}

void Sentence::Draw( Kind kind, Donya::Vector2 ssPos, float degree, float alpha, float scale ) const
{
	int iKind = scast<int>( kind );
	if ( iKind < 0 || KIND_COUNT <= iKind ) { return; }
	// else

	Donya::Sprite::DrawPartExt
	(
		sprite,
		ssPos.x, ssPos.y,
		texPartPos[iKind].x,
		texPartPos[iKind].y,
		texPartSize[iKind].x,
		texPartSize[iKind].y,
		scale, scale,
		degree, alpha
	);
}

void Sentence::LoadParameter( bool isBinary )
{
	Serializer::Extension ext = ( isBinary )
	? Serializer::Extension::BINARY
	: Serializer::Extension::JSON;
	std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

	Serializer seria;
	seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
}

#if USE_IMGUI

void Sentence::SaveParameter()
{
	Serializer::Extension bin  = Serializer::Extension::BINARY;
	Serializer::Extension json = Serializer::Extension::JSON;
	std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
	std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

	Serializer seria;
	seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
	seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
}

void Sentence::UseImGui()
{
	if ( ImGui::BeginIfAllowed() )
	{
		if ( ImGui::TreeNode( u8"�t�h������" ) )
		{
			std::string caption{};

			if ( ImGui::TreeNode( u8"�e�N�X�`�����W" ) )
			{
				for ( int i = 0; i < KIND_COUNT; ++i )
				{
					caption = "���W[" + std::to_string( i ) + "]";
					ImGui::SliderFloat2( Donya::MultiToUTF8( caption ).c_str(), &texPartPos[i].x, 0.0f, 1920.0f );
				}
				ImGui::TreePop();
			}

			if ( ImGui::TreeNode( u8"�e�N�X�`���؂���T�C�Y" ) )
			{
				for ( int i = 0; i < KIND_COUNT; ++i )
				{
					caption = "�T�C�Y[" + std::to_string( i ) + "]";
					ImGui::SliderFloat2( Donya::MultiToUTF8( caption ).c_str(), &texPartSize[i].x, 0.0f, 1920.0f );
				}
				ImGui::TreePop();
			}
			
			if ( ImGui::TreeNode( u8"�t�@�C��" ) )
			{
				static bool isBinary = false;
				if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
				if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
				std::string loadStr{ "�ǂݍ��� " };
				loadStr += ( isBinary ) ? "Binary" : "JSON";

				if ( ImGui::Button( u8"�ۑ�" ) )
				{
					SaveParameter();
				}
				if ( ImGui::Button( Donya::MultiToUTF8( loadStr ).c_str() ) )
				{
					LoadParameter( isBinary );
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}
}

#endif // USE_IMGUI
