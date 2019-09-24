// Prevent DrawText macro.
#define NODRAWTEXT ( true )

#include "SceneTitle.h"

#include <algorithm>
#include <string>
#include <vector>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "Donya/Blend.h"
#include "Donya/GamepadXInput.h"
#include "Donya/Keyboard.h"
#include "Donya/Random.h"
#include "Donya/Serializer.h"
#include "Donya/Sound.h"
#include "Donya/Sprite.h"
#include "Donya/Useful.h"
#include "Donya/UseImgui.h"
#include "Donya/Vector.h"

#include "Common.h"
#include "Fader.h"
#include "FilePath.h"
#include "Music.h"

#undef min
#undef max

struct CerealData
{
	Donya::Vector2 scrollVelocity;

	Donya::Vector2 logoPos;
	Donya::Vector2 playerPos;
	Donya::Vector2 promptPos;
	Donya::Vector2 usageBoardPos;
	Donya::Vector2 usageBoardSize;	// per Pixel.
	Donya::Vector2 usagePos;
	Donya::Vector2 usageScale;
	float usageAlpha;

	std::vector<Donya::Vector2> stagePos;
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( scrollVelocity ),
			CEREAL_NVP( stagePos )
		);

		if ( 1 <= version )
		{
			archive
			(
				CEREAL_NVP( logoPos ),
				CEREAL_NVP( playerPos ),
				CEREAL_NVP( promptPos ),
				CEREAL_NVP( usagePos ),
				CEREAL_NVP( usageScale ),
				CEREAL_NVP( usageAlpha )
			);
		}
		if ( 2 <= version )
		{
			archive
			(
				CEREAL_NVP( usageBoardPos ),
				CEREAL_NVP( usageBoardSize )
			);
		}
	}
public:
	static constexpr const char *SERIAL_ID = "TitleParameter";
};
CEREAL_CLASS_VERSION( CerealData, 2 )

class SceneTitle::Impl
{
public:
	enum class State
	{
		SHOW_TITLE,
		SELECT_STAGE,
	};
public:
	CerealData		cerealData;

	State			status;

	size_t			sprStages;
	size_t			sprLogo;
	size_t			sprUsage;
	size_t			sprUI;
	Anime			animePlayer;

	BG				bg;
	Donya::Vector2	bgScroll;
	Donya::XInput	controller;

	int				stageCount;		// 0-based.
	int				chooseStageNo;	// 0-based.

	int				stageAnimeNo;	// 0 ~ 1.
	int				stageAnimeTimer;// 0-based.

	bool			changeScene;
public:
	Impl() : cerealData(),
		status( State::SHOW_TITLE ),
		sprStages( NULL ), sprLogo( NULL ), sprUsage( NULL ), sprUI( NULL ), animePlayer(),
		bg(),
		bgScroll(),
		controller( Donya::Gamepad::PadNumber::PAD_1 ),
		stageCount( 0 ), chooseStageNo( 0 ),
		stageAnimeNo( 0 ), stageAnimeTimer( 0 ),
		changeScene( false )
	{}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		archive( CEREAL_NVP( cerealData ) );
	}
public:
	void InitAnime()
	{
		using ID	= SpriteAttribute;
		sprStages	= Donya::Sprite::Load( GetSpritePath( ID::SelectableStage ), 16U );
		sprLogo		= Donya::Sprite::Load( GetSpritePath( ID::TitleLogo ), 2U );
		sprUsage	= Donya::Sprite::Load( GetSpritePath( ID::Usage ), 2U );
		sprUI		= Donya::Sprite::Load( GetSpritePath( ID::UI ), 256U );

		animePlayer.Init();
		animePlayer.LoadSheet( GetSpritePath( ID::Player ), 64U );
		animePlayer.SetPartCount( { 3,10 } );
		animePlayer.SetPartSize( { 96, 96 } );

		int rangeFirst = ( Donya::Random::GenerateInt( 2 ) ) ? 3 : 0;
		animePlayer.Reset
		(
			21,
			8,
			rangeFirst,
			rangeFirst + 2,
			true,
			true
		);
	}

	void UpdateAnime()
	{
		constexpr int STAGE_ANIME_COUNT = 2;
		constexpr int STAGE_ANIME_SPEED = 8;
		stageAnimeTimer++;
		if ( STAGE_ANIME_SPEED <= stageAnimeTimer )
		{
			stageAnimeTimer = 0;
			stageAnimeNo++;
			if ( STAGE_ANIME_COUNT <= stageAnimeNo )
			{
				stageAnimeNo = 0;
			}
		}

		animePlayer.Update();
	}

	void DrawAnime()
	{
		if ( status == State::SHOW_TITLE )
		{
			Donya::Sprite::Draw
			(
				sprLogo,
				cerealData.logoPos.x,
				cerealData.logoPos.y
			);

			UI::DrawData promptData = UI::GetDrawData( UI::Kind::PromptInput );
			Donya::Sprite::DrawPart
			(
				sprUI,
				cerealData.promptPos.x,
				cerealData.promptPos.y,
				promptData.texPos.Float().x,
				promptData.texPos.Float().y,
				promptData.texSize.Float().x,
				promptData.texSize.Float().y
			);
		}
		else
		{
			UI::DrawData boardData = UI::GetDrawData( UI::Kind::Board );
			Donya::Sprite::DrawGeneral
			(
				sprUI,
				cerealData.usageBoardPos.x,
				cerealData.usageBoardPos.y,
				cerealData.usageBoardSize.x,
				cerealData.usageBoardSize.y,
				boardData.texPos.Float().x,
				boardData.texPos.Float().y,
				boardData.texSize.Float().x,
				boardData.texSize.Float().y
			);

			Donya::Sprite::DrawExt
			(
				sprUsage,
				cerealData.usagePos.x,
				cerealData.usagePos.y,
				cerealData.usageScale.x,
				cerealData.usageScale.y,
				0.0f,
				cerealData.usageAlpha
			);
		}

		if ( status == State::SELECT_STAGE )
		{
			Donya::Vector2 texSize{ 256.0f, 256.0f };

			auto DrawStage = [&]( size_t stageNo, Donya::Vector2 wsPos, bool isEmphasis )
			{
				float scale = ( isEmphasis ) ? 1.5f : 1.0f;

				Donya::Sprite::DrawPartExt
				(
					sprStages,
					wsPos.x,
					wsPos.y,
					texSize.x * stageAnimeNo,
					texSize.y * stageNo,
					texSize.x,
					texSize.y,
					scale, scale
				);
			};

			for ( size_t i = 0; i < cerealData.stagePos.size(); ++i )
			{
				bool isEmphasis = ( i == chooseStageNo ) ? true : false;
				DrawStage( i, cerealData.stagePos[i], isEmphasis );
			}
		}

		Donya::Vector2 texPos  = animePlayer.CalcTexturePartPos().Float();
		Donya::Vector2 texSize = animePlayer.GetPartSize().Float();
		Donya::Sprite::DrawPart
		(
			animePlayer.GetSpriteIdentifier(),
			cerealData.playerPos.x,
			cerealData.playerPos.y,
			texPos.x,  texPos.y,
			texSize.x, texSize.y
		);
	}

	bool IsTriggerAdvance()
	{
		const std::array<bool, 4> input
		{
			( Donya::Keyboard::Trigger( 'Z' ) ) ? true : false,
			controller.Trigger( Donya::Gamepad::Button::A ),
			controller.Trigger( Donya::Gamepad::Button::START ),
			controller.Trigger( Donya::Gamepad::Button::SELECT )
		};

		for ( const auto &it : input )
		{
			if ( it )
			{
				Donya::Sound::Play( Music::ItemDecision );
				return true;
			}
		}

		return false;
	}
public:
	void LoadParameter( bool isBinary = true )
	{
		Serializer::Extension ext = ( isBinary )
			? Serializer::Extension::BINARY
			: Serializer::Extension::JSON;
		std::string filePath = GenerateSerializePath( CerealData::SERIAL_ID, ext );

		Serializer seria;
		seria.Load( ext, filePath.c_str(), CerealData::SERIAL_ID, cerealData );
	}

#if USE_IMGUI

	void SaveParameter()
	{
		Serializer::Extension bin = Serializer::Extension::BINARY;
		Serializer::Extension json = Serializer::Extension::JSON;
		std::string binPath  = GenerateSerializePath( CerealData::SERIAL_ID, bin );
		std::string jsonPath = GenerateSerializePath( CerealData::SERIAL_ID, json );

		Serializer seria;
		seria.Save( bin,  binPath.c_str(),  CerealData::SERIAL_ID, cerealData );
		seria.Save( json, jsonPath.c_str(), CerealData::SERIAL_ID, cerealData );
	}

	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( u8"タイトル関連" ) )
			{
				/*
					Donya::Vector2 scrollVelocity;

					Donya::Vector2 logoPos;
					Donya::Vector2 playerPos;
					Donya::Vector2 promptPos;
					Donya::Vector2 usageBoardPos;
					Donya::Vector2 usageBoardSize;	// per Pixel.
					Donya::Vector2 usagePos;
					Donya::Vector2 usageScale;
					float usageAlpha;

					std::vector<Donya::Vector2> stagePos;
				*/

				auto ShowVector2 = []( const std::string &str, Donya::Vector2 *pVec, float min, float max )
				{
					ImGui::SliderFloat2( str.c_str(), &( pVec->x ), min, max );
				};

				ShowVector2( Donya::MultiToUTF8( "スクロール速度" ), &cerealData.scrollVelocity, -64.0f, 64.0f );
				ImGui::Text( "" );

				ShowVector2( Donya::MultiToUTF8( "ロゴの位置" ), &cerealData.logoPos, 0.0f, 1920.0f );
				ShowVector2( Donya::MultiToUTF8( "プレイヤーの位置" ), &cerealData.playerPos, 0.0f, 1920.0f );
				ShowVector2( Donya::MultiToUTF8( "「ボタンを押してね！！」の位置" ), &cerealData.promptPos, 0.0f, 1920.0f );
				ImGui::Text( "" );

				ShowVector2( Donya::MultiToUTF8( "操作方法の位置" ), &cerealData.usagePos, 0.0f, 1920.0f );
				ShowVector2( Donya::MultiToUTF8( "操作方法のスケール" ), &cerealData.usageScale, 0.001f, 2.0f );
				ImGui::SliderFloat( u8"操作方法の透明度", &cerealData.usageAlpha, 0.0f, 1.0f );
				ImGui::Text( "" );

				ShowVector2( Donya::MultiToUTF8( "操作方法裏の板の位置" ), &cerealData.usageBoardPos, 0.0f, 1920.0f );
				ShowVector2( Donya::MultiToUTF8( "操作方法裏の板の幅" ), &cerealData.usageBoardSize, 0.0f, 1920.0f );
				ImGui::Text( "" );

				for ( size_t i = 0; i < cerealData.stagePos.size(); ++i )
				{
					std::string str = "ステージ[" + std::to_string( i ) + "]の位置";
					ShowVector2( Donya::MultiToUTF8( str ), &cerealData.stagePos[i], 0.0f, 1920.0f );
				}

				if ( ImGui::TreeNode( u8"ファイル" ) )
				{
					static bool isBinary = true;
					if ( ImGui::RadioButton( "Binary", isBinary ) ) { isBinary = true; }
					if ( ImGui::RadioButton( "JSON", !isBinary ) ) { isBinary = false; }
					std::string loadStr{ "読み込み " };
					loadStr += ( isBinary ) ? "Binary" : "JSON";

					if ( ImGui::Button( u8"保存" ) )
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

			if ( ImGui::TreeNode( u8"コントローラ振動テスト" ) )
			{
				static int frame	= 30;
				static float left	= 0.5f;
				static float right	= 0.5f;
				static float byOne	= 0.5f;

				ImGui::SliderInt( u8"振動フレーム（-1で永続）", &frame, -1, 300 );
				ImGui::SliderFloat( u8"左の速度", &left, 0.0f, 1.0f );
				ImGui::SliderFloat( u8"右の速度", &right, 0.0f, 1.0f );
				ImGui::SliderFloat( u8"単体の速度", &byOne, 0.0f, 1.0f );

				if ( ImGui::Button( u8"スタート（左右）" ) )
				{
					controller.Vibrate( frame, left, right );
				}
				if ( ImGui::Button( u8"スタート（単）" ) )
				{
					controller.Vibrate( frame, byOne );
				}
				if ( ImGui::Button( u8"停止" ) )
				{
					controller.StopVibration();
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}

#endif // USE_IMGUI
};

SceneTitle::SceneTitle() : pImpl( std::make_unique<SceneTitle::Impl>() )
{
	
}
SceneTitle::~SceneTitle()
{
	pImpl.reset( nullptr );
}

void SceneTitle::Init()
{
	Donya::Sound::Play( Music::BGM_Title );

	pImpl->InitAnime();

	pImpl->LoadParameter();

	pImpl->bg.Init( -1 ); // Use title version.
	pImpl->bgScroll = {};

	pImpl->stageCount = CalcStageCount();
	if ( scast<int>( pImpl->cerealData.stagePos.size() ) != pImpl->stageCount )
	{
		pImpl->cerealData.stagePos.resize( pImpl->stageCount );
	}
}

void SceneTitle::Uninit()
{
	Donya::Sound::Stop( Music::BGM_Title );
}

Scene::Result SceneTitle::Update( float elapsedTime )
{
#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->controller.Update();

	switch ( pImpl->status )
	{
	case Impl::State::SHOW_TITLE:	TitleUpdate();	break;
	case Impl::State::SELECT_STAGE:	SelectUpdate();	break;
	default: break;
	}

	pImpl->UpdateAnime();

	return ReturnResult();
}

void SceneTitle::Draw( float elapsedTime )
{
	switch( pImpl->status )
	{
	case Impl::State::SHOW_TITLE:	TitleDraw();	break;
	case Impl::State::SELECT_STAGE:	SelectDraw();	break;
	default: break;
	}
}

void SceneTitle::StartFade()
{
	Fader::Configuration config{};
	config.type = Fader::Type::Scroll;
	config.closeFrame	= 20;
	config.parameter	= Fader::Direction::RIGHT;
	Fader::Get().StartFadeOut( config );
}

void SceneTitle::TitleUpdate()
{
	Scroll();

	if ( !Fader::Get().IsExist() && pImpl->IsTriggerAdvance() )
	{
		StartFade();
	}

	if ( Fader::Get().IsClosed() )
	{
		pImpl->status = Impl::State::SELECT_STAGE;
	}
}
void SceneTitle::SelectUpdate()
{
	Scroll();

	if ( !Fader::Get().IsExist() && !pImpl->changeScene )
	{
		ChooseStage();

		if ( pImpl->IsTriggerAdvance() )
		{
			StartFade();
		}
	}

	if ( Fader::Get().IsClosed() && !pImpl->changeScene )
	{
		pImpl->changeScene = true;
		StoreStageNo();
	}
}

void SceneTitle::TitleDraw()
{
	pImpl->bg.Draw( pImpl->bgScroll );

	pImpl->DrawAnime();
}
void SceneTitle::SelectDraw()
{
	pImpl->bg.Draw( pImpl->bgScroll );

	pImpl->DrawAnime();
}

void SceneTitle::Scroll()
{
	pImpl->bgScroll.x += 3.0f;
}

void SceneTitle::ChooseStage()
{
	bool left{}, right{};
	left  = pImpl->controller.Trigger( Donya::Gamepad::Button::LEFT  ) || Donya::Keyboard::Trigger( VK_LEFT );
	right = pImpl->controller.Trigger( Donya::Gamepad::Button::RIGHT ) || Donya::Keyboard::Trigger( VK_RIGHT );
	if ( !left )
	{
		left  = pImpl->controller.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
	}
	if ( !right )
	{
		right = pImpl->controller.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
	}

	int index = pImpl->chooseStageNo;
	int oldIndex = index;

	if ( left  ) { index--; }
	if ( right ) { index++; }

	index = std::max( 0, std::min( pImpl->stageCount - 1, index ) );

	if ( index != oldIndex )
	{
		Donya::Sound::Play( Music::ItemChoose );
	}

	pImpl->chooseStageNo = index;
}

void SceneTitle::StoreStageNo()
{
	StorageForScene data{};
	data.selectedStageNumber = pImpl->chooseStageNo;

	SetStorage( data );
}

Scene::Result SceneTitle::ReturnResult()
{
#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_BACK ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Logo;
		return change;
	}
	// else
	if ( Donya::Keyboard::Trigger( VK_RETURN ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Game;
		return change;
	}
	// else
#endif // DEBUG_MODE

	if ( pImpl->changeScene )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Game;
		return change;
	}
	// else

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}
