#include "SceneGame.h"

#include <array>
#include <memory>
#include <vector>

#include "BG.h"
#include "Common.h"
#include "Config.h"
#include "Constant.h"
#include "Enemy.h"
#include "Fader.h"
#include "FilePath.h"
#include "GamepadXInput.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Music.h"
#include "Object.h"
#include "Player.h"
#include "Random.h"
#include "ScreenShake.h"
#include "Serializer.h"
#include "Sound.h"
#include "Sprite.h"
#include "Stage.h"
#include "StorageForScene.h"
#include "UI.h"
#include "UseImGui.h"

void PlayBGM( int stageNo )
{
	Music::ID id = Music::TERMINATION_OF_MUSIC_ID;

	switch ( stageNo )
	{
	case 0: id = Music::BGM_Stage0; break;
	case 1: id = Music::BGM_Stage1; break;
	case 2: id = Music::BGM_Stage2; break;
	case 3: id = Music::BGM_Stage3; break;
	case 4: id = Music::BGM_Stage4; break;
	default: return;
	}

	Donya::Sound::Play( id );
}
void StopBGM()
{
	constexpr std::array<Music::ID, 6> BGM
	{
		Music::BGM_Stage0,
		Music::BGM_Stage1,
		Music::BGM_Stage2,
		Music::BGM_Stage3,
		Music::BGM_Stage4,
		Music::BGM_Result,
	};

	for ( size_t i = 0; i < BGM.size(); ++i )
	{
		Donya::Sound::Stop( BGM[i] );
	}
}

Donya::Int2 GetNumberSize()
{
	return { 44, 64 };
}
/// <summary>
/// Returns only the first digit place.
/// </summary>
Donya::Int2 CalcNumberPos( int number )
{
	if ( 10 < number )
	{
		number %= 10;
	}

	Donya::Int2 place{};
	place.x = GetNumberSize().x;
	place.y = 96; // The Y-position of number of texture.

	place.x *= number;

	return place;
}

class Timer
{
public:
	static void DrawTime( size_t spriteID, Donya::Int2 wsPos, const Timer &time, float alpha )
	{
		const Donya::Int2 NUMBERS_SIZE{ 352, 80 }; // Size of [XX:XX:XX].

		auto DrawNumber		= [&]( Donya::Int2 wsPos, int number, int offset )
		{
			Donya::Int2 texPos = CalcNumberPos( number );
			Donya::Int2 texSize = GetNumberSize();

			Donya::Vector2 drawPos
			{
				scast<float>( wsPos.x - ( offset * texSize.x ) ),
				scast<float>( wsPos.y + NUMBERS_SIZE.y )
			};

			Donya::Sprite::DrawPart
			(
				spriteID,
				drawPos.x + ( NUMBERS_SIZE.Float().x * 0.5f ),
				drawPos.y,
				texPos.Float().x,
				texPos.Float().y,
				texSize.Float().x,
				texSize.Float().y,
				0.0f, alpha
			);
		};
		auto DrawColon		= [&]( Donya::Int2 wsPos, int offset )
		{
			Donya::Int2 texPos = CalcNumberPos( 10 );
			Donya::Int2 texSize = GetNumberSize();

			Donya::Vector2 drawPos
			{
				scast<float>( wsPos.x - ( offset * texSize.x ) ),
				scast<float>( wsPos.y + NUMBERS_SIZE.y )
			};

			Donya::Sprite::DrawPart
			(
				spriteID,
				drawPos.x + ( NUMBERS_SIZE.Float().x * 0.5f ),
				drawPos.y,
				texPos.Float().x,
				texPos.Float().y,
				texSize.Float().x,
				texSize.Float().y,
				0.0f, alpha
			);
		};
		auto DrawAllDigit	= [&]( Donya::Int2 wsPos, int number )
		{
			int i = 0;

			while ( 0 < number )
			{
				DrawNumber( wsPos, number % 10, i );
				number /= 10;

				i++;
			}

			while ( i < 2 )
			{
				DrawNumber( wsPos, 0, i );
				i++;
			}
		};
		auto DrawTime		= [&]( Donya::Int2 wsPos, const Timer &time )
		{
			const std::array<int, 3> times
			{
				time.Current(),
				time.Second(),
				time.Minute()
			};

			const int OFFSET = GetNumberSize().x * 3;

			for ( const auto &it : times )
			{
				DrawAllDigit( wsPos, it );

				wsPos.x -= OFFSET;
			}
		};

		DrawTime( wsPos, time );
		DrawColon( wsPos, 2 );
		DrawColon( wsPos, 5 );
	}
private:
	int current;	// 0 ~ 59.
	int second;		// 0 ~ 59.
	int minute;		// 0 ~ 99.
public:
	Timer() : current( 59 ), second( 59 ), minute( 99 ) {}
	Timer( const Timer & ) = default;
	Timer &operator = ( const Timer & ) = default;
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( current ),
			CEREAL_NVP( second ),
			CEREAL_NVP( minute )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP() );
		}
	}
	static constexpr const char *SERIAL_ID = "Timer";
public:
	/// <summary>
	/// If setting -1, that parameter is not change.
	/// </summary>
	void Set( int newMinute = -1, int newSecond = -1, int newCurrent = -1 )
	{
		if ( newMinute != -1 ) { minute = newMinute; }
		if ( newSecond != -1 ) { second = newSecond; }
		if ( newCurrent != -1 ) { current = newCurrent; }
	}

	void Update()
	{
		if ( IsMaxCount() ) { return; }
		// else

		current++;

		if ( 60 <= current )
		{
			current = 0;
			second++;
			if ( 60 <= second )
			{
				second = 0;
				minute++;
				minute = std::min( 99, minute );
			}
		}
	}

	bool IsMaxCount() const
	{
		if ( minute != 99 ) { return false; }
		if ( second != 59 ) { return false; }
		if ( current != 59 ) { return false; }
		return true;
	}

	int Current() const { return current; }
	int Second() const { return second; }
	int Minute() const { return minute; }
};

#pragma region TimerOperators

bool operator <  ( const Timer &L, const Timer &R )
{
	if ( R.Minute() < L.Minute() ) { return false; }
	if ( R.Second() < L.Second() ) { return false; }
	if ( R.Current() <= L.Current() ) { return false; }
	return true;
}
bool operator >  ( const Timer &L, const Timer &R ) { return R < L; }
bool operator <= ( const Timer &L, const Timer &R ) { return !( R < L ); }
bool operator >= ( const Timer &L, const Timer &R ) { return !( L < R ); }

bool operator == ( const Timer &L, const Timer &R ) { return !( L < R ) && !( R > L ); }
bool operator != ( const Timer &L, const Timer &R ) { return !( L == R ); }

// region TimerOperators
#pragma endregion

CEREAL_CLASS_VERSION( Timer, 0 )

class PauseAgent
{
public:
	static bool IsTriggerPauseButton( const Donya::XInput &pController )
	{
		return	(
					Donya::Keyboard::Trigger( 'P' ) ||
					pController.Trigger( Donya::Gamepad::Button::START )
				)
				? true
				: false;
	}
public:
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		Resume,
		ReTry,
	};
private:
	Choice choice;
	SpriteSheet sprUI;
public:
	PauseAgent() : choice( Choice::Resume ), sprUI() {}
public:
	void Init()
	{
		sprUI.LoadSheet( GetSpritePath( SpriteAttribute::UI ), 256U );
	}

	Choice Update( const Donya::XInput &input )
	{
		Choice chosen = Choice::Nil;

		if ( Fader::Get().IsExist() ) { return chosen; }
		// else

		UpdateChooseItem( input );

		if ( IsTriggerPauseButton( input ) )
		{
			Donya::Sound::Play( Music::ItemDecision );
			chosen = Choice::Resume;
		}

		if ( DecideItem( input ) )
		{
			chosen = choice;
		}

		return chosen;
	}

	void Draw()
	{
		DrawDarkness();

		DrawItems();
	}
private:
	void UpdateChooseItem( const Donya::XInput &input )
	{
		bool left{}, right{};
		left  = input.Trigger( Donya::Gamepad::Button::LEFT  ) || Donya::Keyboard::Trigger( VK_LEFT );
		right = input.Trigger( Donya::Gamepad::Button::RIGHT ) || Donya::Keyboard::Trigger( VK_RIGHT );
		if ( !left )
		{
			left  = input.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
		}
		if ( !right )
		{
			right = input.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
		}

		int index = scast<int>( choice );
		int oldIndex = index;

		if ( left ) { index--; }
		if ( right ) { index++; }

		index = std::max( 0, std::min( scast<int>( Choice::ReTry ), index ) );

		if ( index != oldIndex )
		{
			Donya::Sound::Play( Music::ItemChoose );
		}

		choice = scast<Choice>( index );
	}

	bool DecideItem( const Donya::XInput &input )
	{
		if ( input.Trigger( Donya::Gamepad::Button::A ) || Donya::Keyboard::Trigger( 'Z' ) )
		{
			Donya::Sound::Play( Music::ItemDecision );
			return true;
		}
		// else

		return false;
	}
private:
	void DrawDarkness()
	{
		Donya::Sprite::DrawRect
		(
			Common::HalfScreenWidthF(),
			Common::HalfScreenHeightF(),
			Common::ScreenWidthF(),
			Common::ScreenHeightF(),
			Donya::Sprite::Color::BLACK, 0.5f
		);
	}

	void DrawBackBoard( Donya::Vector2 wsPos, Donya::Int2 size )
	{
		auto drawData = UI::GetDrawData( UI::Kind::Board );
		Donya::Vector2 texPos = drawData.texPos.Float();
		Donya::Vector2 texSize = drawData.texSize.Float();

		Donya::Sprite::DrawGeneral
		(
			sprUI.GetSpriteIdentifier(),
			wsPos.x,
			wsPos.y,
			size.Float().x * 1.2f,
			size.Float().y * 1.2f,
			texPos.x,
			texPos.y,
			texSize.x,
			texSize.y
		);
	}
	void DrawItem( const UI::DrawData &drawData, const Donya::Vector2 &pos, bool isEmphasis = false )
	{
		constexpr float MAGNI = 1.5f;
		float magni = ( isEmphasis ) ? MAGNI : 1.0f;

		Donya::Int2 magniSize
		{
			scast<int>( drawData.texSize.x * magni ),
			scast<int>( drawData.texSize.y * magni )
		};
		DrawBackBoard( pos, magniSize );

		Donya::Sprite::DrawPartExt
		(
			sprUI.GetSpriteIdentifier(),
			pos.x, pos.y,
			drawData.texPos.Float().x,
			drawData.texPos.Float().y,
			drawData.texSize.Float().x,
			drawData.texSize.Float().y,
			magni, magni
		);
	};
	void DrawItems()
	{
		constexpr size_t ITEM_COUNT = 4;

		const std::array<UI::DrawData, ITEM_COUNT> drawData // Left start.
		{
			UI::GetDrawData( UI::Kind::BackToTitle ),
			UI::GetDrawData( UI::Kind::BackToGame ),
			UI::GetDrawData( UI::Kind::ReTry ),
			UI::GetDrawData( UI::Kind::Pause )
		};
		const Donya::Vector2 TOP_POS{ 960.0f, 320.0f };
		const Donya::Vector2 BASE_POS{ 320.0f, 940.0f };
		const Donya::Vector2 TO_CENTER{ 680.0f, 0.0f };
		const Donya::Vector2 TO_RIGHT{ 1300.0f, 0.0f };
		const std::array<Donya::Vector2, ITEM_COUNT> itemPoses // Left start.
		{
			BASE_POS,
			BASE_POS + TO_CENTER,
			BASE_POS + TO_RIGHT,
			TOP_POS,
		};

		for ( size_t i = 0; i < ITEM_COUNT; ++i )
		{
			bool isEmphasis = ( i == scast<size_t>( choice ) || i == ITEM_COUNT - 1 ) ? true : false;
			DrawItem( drawData[i], itemPoses[i], isEmphasis );
		}
	}
};

class ResultPerformer
{
public:
	enum class State
	{
		Appear,
		AskNext,
		ApplyFade
	};
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		GoToNextStage,
		ReTry,
	};
private:
	const Donya::Gamepad::Button ADVANCE_BUTTON;

	State	status;

	SpriteSheet sprBoard;
	SpriteSheet sprUI;

	Timer	time;
	Timer	highScore;

	Choice	choice;

	int		stageNo;
	int		maxStageCount;
	int		deathCount;

	float	alpha;	// 0.0f ~ 1.0f.
public:
	ResultPerformer() : ADVANCE_BUTTON( Donya::Gamepad::Button::A ),
		status( State::Appear ),
		sprBoard(), sprUI(),
		time(), highScore(),
		choice( Choice::GoToNextStage ),
		stageNo( 0 ), maxStageCount( 0 ), deathCount( 0 ),
		alpha( 0 )
	{}
public:
	void Init( int stageNumber, int stageCount, int currentDeathCount, const Timer &currentTime, const Timer &currentHighScore )
	{
		StopBGM();
		Donya::Sound::Play( Music::BGM_Result );

		sprUI.LoadSheet( GetSpritePath( SpriteAttribute::UI ), 256U );
		sprBoard.LoadSheet( GetSpritePath( SpriteAttribute::ScoreBoard ), 4U );

		time		= currentTime;
		highScore	= currentHighScore;
		stageNo		= stageNumber;
		maxStageCount = stageCount;
		deathCount	= currentDeathCount;
	}

	Choice Update( const Donya::XInput &input )
	{
		Choice chosen = Nil;

		switch ( status )
		{
		case State::Appear:		AppearUpdate( input );				break;
		case State::AskNext:	chosen = AskNextUpdate( input );	break;
		case State::ApplyFade:	break;
		default: break;
		}

		return chosen;
	}

	void Draw()
	{
		DrawBoard();

		DrawScores();

		DrawItems();
	}
private:
	bool IsLastStage() const
	{
		return ( stageNo == ( maxStageCount - 1 ) ) ? true : false;
	}

	void UpdateChooseItem( const Donya::XInput &input )
	{
		bool left{}, right{};
		left  = input.Trigger( Donya::Gamepad::Button::LEFT  ) || Donya::Keyboard::Trigger( VK_LEFT  );
		right = input.Trigger( Donya::Gamepad::Button::RIGHT ) || Donya::Keyboard::Trigger( VK_RIGHT );
		if ( !left )
		{
			left = input.TriggerStick( Donya::Gamepad::StickDirection::LEFT );
		}
		if ( !right )
		{
			right = input.TriggerStick( Donya::Gamepad::StickDirection::RIGHT );
		}

		int index = scast<int>( choice );
		int oldIndex = index;

		if ( left  ) { index--; }
		if ( right ) { index++; }

		if ( IsLastStage() && index == 1 )
		{
			if ( left  ) { index--; }
			if ( right ) { index++; }
		}

		index = std::max( 0, std::min( scast<int>( Choice::ReTry ), index ) );

		if ( index != oldIndex )
		{
			Donya::Sound::Play( Music::ItemChoose );
		}

		choice = scast<Choice>( index );
	}

	void AppearUpdate( const Donya::XInput &input )
	{
		if ( alpha < 1.0f )
		{
			constexpr float FADE_IN = 1.0f / 30.0f/*Half-Frame*/;
			alpha += FADE_IN;
		}
		
		if ( 0.9f <= alpha )
		{
			alpha = 1.0f;
			status = State::AskNext;
		}

		constexpr float ALLOW_INPUT_ALPHA = 0.3f;
		if ( ALLOW_INPUT_ALPHA <= alpha )
		{
			UpdateChooseItem( input );
		}
	}
	Choice AskNextUpdate( const Donya::XInput &input )
	{
		Choice rv = Choice::Nil;

		UpdateChooseItem( input );

		bool canAdvance = true;
		if ( IsLastStage() )
		{
			if ( choice == Choice::GoToNextStage )
			{
				canAdvance = false;
			}
		}

		bool userInput = ( input.Trigger( ADVANCE_BUTTON ) || Donya::Keyboard::Trigger( 'Z' ) );
		if ( canAdvance && userInput )
		{
			status = State::ApplyFade;
			rv = choice;

			Donya::Sound::Play( Music::ItemDecision );
		}

		return rv;
	}

	void DrawBoard()
	{
		Donya::Sprite::Draw
		(
			sprBoard.GetSpriteIdentifier(),
			Common::HalfScreenWidthF(),
			Common::HalfScreenHeightF(),
			0.0f, alpha
		);
	}
	void DrawBackBoard( Donya::Vector2 wsPos, Donya::Int2 size )
	{
		auto drawData = UI::GetDrawData( UI::Kind::Board );
		Donya::Vector2 texPos  = drawData.texPos.Float();
		Donya::Vector2 texSize = drawData.texSize.Float();

		Donya::Sprite::DrawGeneral
		(
			sprUI.GetSpriteIdentifier(),
			wsPos.x,
			wsPos.y,
			size.Float().x * 1.2f,
			size.Float().y * 1.2f,
			texPos.x,
			texPos.y,
			texSize.x,
			texSize.y,
			0.0f, Donya::Sprite::Origin::CENTER,
			alpha
		);
	}
	void DrawItem( const UI::DrawData &drawData, const Donya::Vector2 &pos, bool isEmphasis = false )
	{
		constexpr float MAGNI = 1.5f;
		float magni = ( isEmphasis ) ? MAGNI : 1.0f;

		Donya::Int2 magniSize
		{
			scast<int>( drawData.texSize.x * magni ),
			scast<int>( drawData.texSize.y * magni )
		};
		DrawBackBoard( pos, magniSize );

		Donya::Sprite::DrawPartExt
		(
			sprUI.GetSpriteIdentifier(),
			pos.x, pos.y,
			drawData.texPos.Float().x,
			drawData.texPos.Float().y,
			drawData.texSize.Float().x,
			drawData.texSize.Float().y,
			magni, magni,
			0.0f, alpha
		);
	};
	void DrawScores()
	{
		const Donya::Int2 SCORE_POS{ 940, 368 };
		
		auto texScore = UI::GetDrawData( UI::Kind::TimeJP );
		DrawItem( texScore, SCORE_POS.Float() );
		Timer::DrawTime( sprUI.GetSpriteIdentifier(), SCORE_POS, time, alpha );

		const Donya::Int2 SHIFT{ 0, 80 * 2 };
		auto texHighScore = UI::GetDrawData( UI::Kind::BestTime );
		DrawItem( texHighScore, ( SCORE_POS + SHIFT ).Float() );
		Timer::DrawTime( sprUI.GetSpriteIdentifier(), SCORE_POS + SHIFT, highScore, alpha );
	}
	void DrawItems()
	{
		constexpr size_t ITEM_COUNT = 4;

		const std::array<UI::DrawData, ITEM_COUNT> drawData // Left start.
		{
			UI::GetDrawData( UI::Kind::BackToTitle ),
			UI::GetDrawData( UI::Kind::GoToNextStage ),
			UI::GetDrawData( UI::Kind::ReTry ),
			UI::GetDrawData( UI::Kind::DeathCount )
		};
		const Donya::Vector2 BASE_POS{ 320.0f, 940.0f };
		const Donya::Vector2 TO_CENTER{ 680.0f, 0.0f };
		const Donya::Vector2 TO_RIGHT{ 1300.0f, 0.0f };
		const Donya::Vector2 DEATH_SENTENCE{ 940.0f, 200.0f };
		const Donya::Vector2 DEATH_NUMBER{ 984.0f, 280.0f };
		const std::array<Donya::Vector2, ITEM_COUNT> itemPoses // Left start.
		{
			BASE_POS,
			BASE_POS + TO_CENTER,
			BASE_POS + TO_RIGHT,
			DEATH_SENTENCE,
		};

		for ( size_t i = 0; i < ITEM_COUNT; ++i )
		{
			// Prevent show "go to next stage" when at last stage.
			if ( IsLastStage() && i == 1 ) { continue; }
			// else
			bool isEmphasis = ( i == scast<size_t>( choice ) ) ? true : false;
			DrawItem( drawData[i], itemPoses[i], isEmphasis );
		}

		auto DrawNumber = [&]( Donya::Vector2 wsPos, int number, int offset )
		{
			if ( 9 < number )
			{
				number %= 10;
			}

			Donya::Int2 texPos  = CalcNumberPos( number );
			Donya::Int2 texSize = GetNumberSize();

			Donya::Vector2 drawPos
			{
				wsPos.x - scast<float>( offset * texSize.x ),
				wsPos.y
			};

			Donya::Sprite::DrawPart
			(
				sprUI.GetSpriteIdentifier(),
				drawPos.x,
				drawPos.y,
				texPos.Float().x,
				texPos.Float().y,
				texSize.Float().x,
				texSize.Float().y,
				0.0f, alpha
			);
		};
		auto DrawAllDigit = [&]( Donya::Vector2 wsPos, int number )
		{
			int i = 0;

			while ( 0 < number )
			{
				DrawNumber( wsPos, number % 10, i );
				number /= 10;

				i++;
			}

			while ( i < 2 )
			{
				DrawNumber( wsPos, 0, i );
				i++;
			}
		};

		DrawAllDigit( DEATH_NUMBER, deathCount );
	}
public:
	static void Create( std::unique_ptr<ResultPerformer> *pSaucer, int stageNo, int maxStageCount, int deathCount, const Timer &currentTime, const Timer &currentHighScore )
	{
		if ( !pSaucer ) { return; }
		// else

		*pSaucer = std::make_unique<ResultPerformer>();
		( *pSaucer )->Init( stageNo, maxStageCount, deathCount, currentTime, currentHighScore );
	}
};

struct SceneGame::Impl
{
	int					currentStageNo;	// 0-based.
	int					stageCount;		// 0-based.
	int					deathCount;		// 0-based.
	SpriteSheet			sprUI;
	BG					bg;
	EnemyBundle			enemies;
	ObjectBundle		objects;
	Player				player;
	Stage				stages;
	std::vector<Timer>	timers;
	std::vector<Timer>	highScores;
	Donya::XInput		controller;
	std::unique_ptr<PauseAgent>			pPause;
	std::unique_ptr<ResultPerformer>	pResultPerformer;
	Scene::Type			nextScene;
	bool				didMiss;
	bool				didGoal;
	bool				hasRespawnPoint;
	bool				shouldChangeScene;
public:
	Impl() : currentStageNo( 0 ), stageCount( 0 ), deathCount( 0 ),
		sprUI(),
		bg(), enemies(), objects(), player(), stages(),
		timers(), highScores(),
		controller( Donya::XInput::PadNumber::PAD_1 ),
		pPause( nullptr ), pResultPerformer( nullptr ),
		nextScene( Scene::Type::Null ),
		didMiss( false ), didGoal( false ), hasRespawnPoint( false ),
		shouldChangeScene( false )
	{}
	~Impl()
	{
		timers.clear();
		timers.shrink_to_fit();
	}
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive )
	{
		archive
		(
			CEREAL_NVP( highScores )
		);
	}
	static constexpr const char *SERIAL_ID = "Game";
public:
	bool IsAttack() const
	{
		return
			controller.Trigger( Donya::XInput::Button::X )
			|| controller.Trigger( Donya::XInput::Button::Y )
			|| Donya::Keyboard::Trigger( 'Z' )
			;
	}
	bool IsJump() const
	{
		return
			controller.Trigger( Donya::XInput::Button::A )
			|| controller.Trigger( Donya::XInput::Button::B )
			|| Donya::Keyboard::Trigger( VK_LSHIFT )
			;
	}
public:
	void UpdateHighScoreIfFasted( int stageNo )
	{
		if ( stageCount <= stageNo ) { return; }
		// else

		auto &current	= timers[stageNo];
		auto &highScore	= highScores[stageNo];

		if ( current <= highScore )
		{
			highScore = current;
			SaveParameter();
		}
	}
	void LoadParameter( bool isBinary = true )
	{
		Serializer::Extension ext =	( isBinary )
									? Serializer::Extension::BINARY
									: Serializer::Extension::JSON;
		std::string filePath = GenerateSerializePath( SERIAL_ID, ext );

		Serializer seria;
		seria.Load( ext, filePath.c_str(), SERIAL_ID, *this );
	}
	void SaveParameter()
	{
		Serializer::Extension bin  = Serializer::Extension::BINARY;
		Serializer::Extension json = Serializer::Extension::JSON;
		std::string binPath  = GenerateSerializePath( SERIAL_ID, bin );
		std::string jsonPath = GenerateSerializePath( SERIAL_ID, json );

		Serializer seria;
		seria.Save( bin,  binPath.c_str(),  SERIAL_ID, *this );
		seria.Save( json, jsonPath.c_str(), SERIAL_ID, *this );
	}
public:
	void StartFade()
	{
		Fader::Configuration config{};
		config.type = Fader::Type::Scroll;
		config.closeFrame	= 20;
		config.parameter	= Fader::Direction::RIGHT;
		Fader::Get().StartFadeOut( config );
	};

	ResultPerformer::Choice ResultUpdate()
	{
		using Choice = ResultPerformer::Choice;

		Choice returnResult = Choice::Nil;

		auto result = pResultPerformer->Update( controller );
		if ( result != Choice::Nil )
		{
			switch ( result )
			{
			case Choice::GoToNextStage:
				currentStageNo++;

				if ( stageCount <= currentStageNo )
				{
					currentStageNo = 0;
					shouldChangeScene = true;
					nextScene = Scene::Type::Title;
				}

				returnResult = Choice::GoToNextStage;
				StartFade();
				break;
			case Choice::ReTry:
				returnResult = Choice::GoToNextStage;
				StartFade();
				break;
			case Choice::BackToTitle:
				shouldChangeScene = true;
				nextScene = Scene::Type::Title;
				StartFade();
				break;
			default:
				break;
			}
		}

		Player::Input input{};
		input.stick = Donya::Vector2::Right();
		player.Update( input );
		enemies.Update();

		return returnResult;
	}
public:
#if USE_IMGUI

	void UseImGui()
	{
		if ( ImGui::BeginIfAllowed() )
		{
			if ( ImGui::TreeNode( "Game" ) )
			{
				if ( ImGui::TreeNode( "Xbox Input" ) )
				{
					ImGui::Text( "Input Check." );

					static Donya::XInput controller{ Donya::XInput::PadNumber::PAD_1 };
					controller.Update();

					using Button = Donya::XInput::Button;
					std::vector<std::string> inputs
					{
						"UP : " + std::to_string( controller.Press( Button::UP ) ),
						"DOWN : " + std::to_string( controller.Press( Button::DOWN ) ),
						"LEFT : " + std::to_string( controller.Press( Button::LEFT ) ),
						"RIGHT : " + std::to_string( controller.Press( Button::RIGHT ) ),
						"START : " + std::to_string( controller.Press( Button::START ) ),
						"SELECT : " + std::to_string( controller.Press( Button::SELECT ) ),
						"PRESS_L : " + std::to_string( controller.Press( Button::PRESS_L ) ),
						"PRESS_R : " + std::to_string( controller.Press( Button::PRESS_R ) ),
						"LB : " + std::to_string( controller.Press( Button::LB ) ),
						"RB : " + std::to_string( controller.Press( Button::RB ) ),
						"A : " + std::to_string( controller.Press( Button::A ) ),
						"B : " + std::to_string( controller.Press( Button::B ) ),
						"X : " + std::to_string( controller.Press( Button::X ) ),
						"Y : " + std::to_string( controller.Press( Button::Y ) ),
						"LT : " + std::to_string( controller.Press( Button::LT ) ),
						"RT : " + std::to_string( controller.Press( Button::RT ) ),
						"Stick L X : " + std::to_string( controller.LeftStick().x ),
						"Stick L Y : " + std::to_string( controller.LeftStick().y ),
						"Stick R X : " + std::to_string( controller.RightStick().x ),
						"Stick R Y : " + std::to_string( controller.RightStick().y ),
					};

					auto ShowString = []( const std::string &str )
					{
						ImGui::Text( str.c_str() );
					};
					for ( size_t i = 0; i < inputs.size(); ++i )
					{
						ShowString( inputs[i] );
					}

					ImGui::TreePop();
				}

				ImGui::Text( "" );

				auto ShowTime = []( std::string strUTF8, const Timer &timer )
				{
					ImGui::Text
					(
						strUTF8.c_str(),
						timer.Minute(),
						timer.Second(),
						timer.Current()
					);
				};

				ShowTime
				(
					Donya::MultiToUTF8( "現在のタイム[%d:%d:%d]" ),
					timers[currentStageNo]
				);
				for ( size_t i = 0; i < highScores.size(); ++i )
				{
					ShowTime
					(
						Donya::MultiToUTF8( "ハイスコア[" + std::to_string( i ) + "][%d:%d:%d]" ),
						highScores[i]
					);
				}

				if ( ImGui::TreeNode( u8"ファイル（タイマー）" ) )
				{
					static bool isBinary = false;
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

			ImGui::End();
		}
	}

#endif // USE_IMGUI
};

SceneGame::SceneGame() : pImpl( std::make_unique<Impl>() )
{

}
SceneGame::~SceneGame()
{
	pImpl.reset( nullptr );
}

void SceneGame::Init()
{
	pImpl->stageCount = CalcStageCount();

	pImpl->sprUI.LoadSheet( GetSpritePath( SpriteAttribute::UI ), 256U );

	pImpl->LoadParameter(); // Load highScores.
	pImpl->timers.resize( pImpl->stageCount );
	if ( pImpl->highScores.size() != pImpl->timers.size() )
	{
		pImpl->highScores.resize( pImpl->stageCount );
	}

	if ( HasStorage() )
	{
		auto pStorage = GetStorageOrNull();
		if ( pStorage != nullptr )
		{
			pImpl->currentStageNo = pStorage->selectedStageNumber;
		}
	}

	InitPerStage( pImpl->currentStageNo, /* resetTimeAndBGM = */true );
}

void SceneGame::InitPerStage( int stageNo, bool resetTimeAndBGM, bool restartFromRespawnPoint )
{
	if ( resetTimeAndBGM )
	{
		StopBGM();
		PlayBGM( stageNo );
		pImpl->timers[stageNo].Set( 0, 0, 0 );

		pImpl->deathCount = 0;
	}

	pImpl->didMiss = false;
	pImpl->didGoal = false;
	pImpl->hasRespawnPoint = false;
	pImpl->pPause.reset( nullptr );
	pImpl->pResultPerformer.reset( nullptr );

	StageAccessor::Init();
	ObjectAccessor::Init();
	Config::Init();


	// I must load stage before another class, because they using size of mapchip.
	pImpl->stages.Init( stageNo );
	StageAccessor::RegisterStage( stageNo, &pImpl->stages );


	Config::LoadFile( stageNo );

	pImpl->bg.Init( stageNo );

	pImpl->objects.Init( stageNo );// I must call this after register stage, because it using size of mapchip.
	ObjectAccessor::RegisterObjects( stageNo, &pImpl->objects );

	pImpl->enemies.Init( stageNo ); // I must call this a

	pImpl->player.Init( stageNo, restartFromRespawnPoint );
}

void SceneGame::Uninit()
{
	pImpl->SaveParameter();

	Donya::ScreenShake::StopX();
	Donya::ScreenShake::StopY();
	StopBGM();
}

Donya::Vector2 ToEightDirection( Donya::Vector2 stick )
{
	if ( stick.IsZero() ) { return Donya::Vector2::Zero(); }
	// else

	constexpr float DIFF = 45.0f;

	float border = DIFF * 0.5f;
	float degree = stick.Degree();

	int sign = ( degree < 0.0f ) ? -1 : 1;
	int i = 0; // 0 ~ 4
	for ( ; border < 180.0f; border += DIFF )
	{
		// Calc area of 8-direction.

		if ( degree * sign <= border )
		{
			break;
		}
		// else
		i++;
	}

	const std::array<Donya::Vector2, 9> ANGLES
	{
		Donya::Vector2{ -1.0f,  0.0f }, // LEFT
		Donya::Vector2{ -1.0f, -1.0f }, // UP-LEFT
		Donya::Vector2{  0.0f, -1.0f }, // UP
		Donya::Vector2{  1.0f, -1.0f }, // RIGHT-UP
		/*Center*/Donya::Vector2{  1.0f,  0.0f }, // RIGHT
		Donya::Vector2{  1.0f,  1.0f }, // RIGHT-BOTTOM
		Donya::Vector2{  0.0f,  1.0f }, // BOTTOM
		Donya::Vector2{ -1.0f,  1.0f }, // LEFT-BOTTOM
		Donya::Vector2{ -1.0f,  0.0f }, // LEFT
	};

	size_t index = ( ANGLES.size() >> 1 ) + ( i * sign );
	Donya::Vector2 dir = ANGLES[index];
	dir.Normalize();
	return dir;
}
Player::Input MakeInputFromStick( Donya::Vector2 stick )
{
	Player::Input input{};
	input.stick = stick;
	input.stick.Normalize();
	input.stick.y *= -1; // In 2D, the Y direction of screen space is inverse.

	return input;
}

Scene::Result SceneGame::Update( float elapsedTime )
{
#if DEBUG_MODE

	if ( Donya::Keyboard::Press( VK_LCONTROL ) || Donya::Keyboard::Press( VK_RCONTROL ) )
	{
		int stageNo = -1;
		if ( Donya::Keyboard::Trigger( '1' ) ) { stageNo = 0; }
		if ( Donya::Keyboard::Trigger( '2' ) ) { stageNo = 1; }
		if ( Donya::Keyboard::Trigger( '3' ) ) { stageNo = 2; }
		if ( Donya::Keyboard::Trigger( '4' ) ) { stageNo = 3; }
		if ( Donya::Keyboard::Trigger( '5' ) ) { stageNo = 4; }

		if ( stageNo != -1 )
		{
			pImpl->currentStageNo = stageNo;
			InitPerStage( pImpl->currentStageNo, /* startRespawnPoint = */false );
		}
	}

#endif // DEBUG_MODE
#if USE_IMGUI

	pImpl->UseImGui();

#endif // USE_IMGUI

	pImpl->controller.Update();

	auto PauseUpdate = [&]()
	{
		if ( pImpl->pPause )
		{
			auto result = pImpl->pPause->Update( pImpl->controller );
			if ( result != PauseAgent::Choice::Nil )
			{
				switch ( result )
				{
				case PauseAgent::BackToTitle:
					pImpl->shouldChangeScene = true;
					pImpl->nextScene = Scene::Type::Title;
					pImpl->StartFade();
					break;
				case PauseAgent::Resume:
					pImpl->pPause.reset( nullptr );
					break;
				case PauseAgent::ReTry:
					InitPerStage( pImpl->currentStageNo, /* resetTimeAndBGM = */true );
					break;
				default: break;
				}
			}
		}
		else // This "else" is necessary. that prevent continuously pause after resume.
		{
			if	(
					PauseAgent::IsTriggerPauseButton( pImpl->controller )
					&& !pImpl->pResultPerformer
					&& !Fader::Get().IsExist()
				)
			{
				pImpl->pPause = std::make_unique<PauseAgent>();
				pImpl->pPause->Init();

				Donya::Sound::Play( Music::ItemDecision );
			}
		}
	};
	PauseUpdate();

	if ( pImpl->pPause )
	{
		return ReturnResult();
	}
	// else

	if ( Fader::Get().IsClosed() )
	{
		if ( pImpl->didGoal )
		{
			InitPerStage( pImpl->currentStageNo, /* resetTimeAndBGM = */true );
		}
		if ( pImpl->didMiss )
		{
			InitPerStage( pImpl->currentStageNo, /* resetTimeAndBGM = */false, /* restartFromRespawnPoint = */pImpl->hasRespawnPoint );
		}
	}
	
	if ( pImpl->pResultPerformer )
	{
		bool shouldReturn = true;
		
		auto result = pImpl->ResultUpdate();
		if ( result != ResultPerformer::Choice::Nil )
		{
			if ( result != ResultPerformer::Choice::BackToTitle )
			{
				shouldReturn = false;
			}
			if ( pImpl->shouldChangeScene )
			{
				shouldReturn = true;
			}
		}

		if ( Fader::Get().IsExist() ) { shouldReturn = true; }

		if ( shouldReturn ) { return ReturnResult(); }
		// else
	}

	pImpl->timers[pImpl->currentStageNo].Update();

	pImpl->objects.Update();

	pImpl->enemies.Update();

	auto UpdatePlayer = [&]()
	{
		Player::Input input{};
		
		{
			Donya::Vector2 key{};
			if ( Donya::Keyboard::Press( VK_RIGHT ) ) { key.x += +1.0f; }
			if ( Donya::Keyboard::Press( VK_LEFT  ) ) { key.x += -1.0f; }
			if ( Donya::Keyboard::Press( VK_UP    ) ) { key.y += -1.0f; }
			if ( Donya::Keyboard::Press( VK_DOWN  ) ) { key.y += +1.0f; }
			key.Normalize();

			input.stick = key;
		}
		
		if ( pImpl->controller.IsConnected() )
		{
			// Donya::Vector2 dir = ToEightDirection( pImpl->controller.LeftStick() );
			Donya::Vector2 dir = pImpl->controller.LeftStick();
			input = MakeInputFromStick( dir );
		}

		input.doAttack	= pImpl->IsAttack();
		input.doJump	= pImpl->IsJump();

		pImpl->player.Update( input );
	};
	UpdatePlayer();

	if ( pImpl->player.DidMiss() && !Fader::Get().IsExist() )
	{
		pImpl->didMiss = true;
		pImpl->deathCount++;
		pImpl->StartFade();
	}

	DoCollisionDetection();

	return ReturnResult();
}

void SceneGame::Draw( float elapsedTime )
{
	Donya::Vector2 camera = pImpl->player.GetCameraPos();

	pImpl->bg.Draw( camera );

	pImpl->stages.Draw( camera );

	pImpl->objects.Draw( camera );

	Config::Draw( pImpl->currentStageNo, camera, pImpl->hasRespawnPoint );

	auto DrawCurrentTime = [&]( Donya::Int2 wsBoardPos, Donya::Int2 wsTimePos )
	{
		auto boardData = UI::GetDrawData( UI::Kind::Board );
		Donya::Sprite::DrawPartExt
		(
			pImpl->sprUI.GetSpriteIdentifier(),
			wsBoardPos.Float().x,
			wsBoardPos.Float().y,
			boardData.texPos.Float().x,
			boardData.texPos.Float().y,
			boardData.texSize.Float().x,
			boardData.texSize.Float().y,
			1.5f, 1.0f
		);

		auto sentenceData = UI::GetDrawData( UI::Kind::Time );
		Donya::Sprite::DrawPartExt
		(
			pImpl->sprUI.GetSpriteIdentifier(),
			wsBoardPos.Float().x - 180.0f,
			wsBoardPos.Float().y,
			sentenceData.texPos.Float().x,
			sentenceData.texPos.Float().y,
			sentenceData.texSize.Float().x,
			sentenceData.texSize.Float().y,
			1.0f, 1.0f
		);

		Timer::DrawTime
		(
			pImpl->sprUI.GetSpriteIdentifier(),
			wsTimePos,
			pImpl->timers[pImpl->currentStageNo],
			1.0f 
		);
	};

	const Donya::Int2 boardPos{ 390, 80 }, timePos{ 512, 0 };
	DrawCurrentTime( boardPos, timePos );

	pImpl->enemies.Draw( camera );

	Donya::Vector2 dir  = ToEightDirection( pImpl->controller.LeftStick() );
	Player::Input input = MakeInputFromStick( dir );
	pImpl->player.Draw( input );

	if ( pImpl->pPause )
	{
		pImpl->pPause->Draw();
	}
	if ( pImpl->pResultPerformer )
	{
		pImpl->pResultPerformer->Draw();
	}
}

void SceneGame::DoCollisionDetection()
{
	constexpr size_t failedValue = UINT_MAX;

	auto FindCollidingObject = [&]( int stageNo, bool vsDangerObj, const Box *pOtherBox, const Circle *pOtherCircle )->size_t
	{
		if ( pOtherBox && pOtherCircle ) { return failedValue; }

		if ( !ObjectAccessor::HasObjects( stageNo ) ) { return failedValue; }
		// else

		auto pObjects = ObjectAccessor::GetObjectsOrNull( stageNo );
		if ( !pObjects ) { return failedValue; }
		// else

		Box object{};
		size_t count = pObjects->GetObjectCount();
		for ( size_t i = 0; i < count; ++i )
		{
			auto pObject = pObjects->GetObjectOrNull( i );
			if ( !pObject ) { continue; }
			// else

			if ( Object::IsDanger( pObject->GetKind() ) != vsDangerObj ) { continue; }
			// else

			object = pObject->GetWorldSpaceHitBox();

			if ( pOtherBox && Box::IsHitBox( object, *pOtherBox ) )
			{
				return i;
			}
			if ( pOtherCircle && Box::IsHitCircle( object, *pOtherCircle ) )
			{
				return i;
			}
		}

		return failedValue;
	};

	auto PlayerAttack = [&]()
	{
		auto VS_Enemies = [&]( const Circle &other, Donya::Vector2 impactVelocity )
		{
			if ( !other.exist ) { return; }
			// else

			Circle enemy{};
			size_t count = pImpl->enemies.GetEnemyCount();
			for ( size_t i = 0; i < count; ++i )
			{
				enemy = pImpl->enemies.GetWorldSpaceHitBox( i );
				if ( Circle::IsHitCircle( enemy, other ) )
				{
					pImpl->enemies.ReceiveImpact( i, impactVelocity );

					auto kind = Donya::ScreenShake::Kind::PERMANENCE;
					auto param = EnemyParameter::Get()->Open();
					Donya::ScreenShake::SetX( kind, param.shake.x, NULL, 0.25f, 0.05f );
					Donya::ScreenShake::SetY( kind, param.shake.y, NULL, 0.25f, 0.05f );
					pImpl->player.SetHitStop( param.hitStopFrame );

					pImpl->controller.Vibrate
					(
						param.vibrateTime,
						param.vibrateStrengthLeft,
						param.vibrateStrengthRight
					);

					Donya::Sound::Play( Music::PlayerAtkHit );
				}
			}
		};

		auto atkHitBoxes = pImpl->player.RequireAttackHitBoxes();
		Donya::Vector2 impactVelocity = pImpl->player.GetAttackDirection() * pImpl->player.GetAttackSpeed();
		for ( const auto it : atkHitBoxes )
		{
			VS_Enemies( it, impactVelocity );

			size_t result = FindCollidingObject( pImpl->currentStageNo, false, nullptr, &it );
			if ( result != failedValue )
			{
				ObjectAccessor::ReceiveAttack( pImpl->currentStageNo, result );

				auto kind = Donya::ScreenShake::Kind::PERMANENCE;
				auto param = EnemyParameter::Get()->Open();
				Donya::ScreenShake::SetX( kind, param.shake.x, NULL, 0.25f, 0.05f );
				Donya::ScreenShake::SetY( kind, param.shake.y, NULL, 0.25f, 0.05f );
				pImpl->player.SetHitStop( param.hitStopFrame );

				pImpl->controller.Vibrate
				(
					param.vibrateTime,
					param.vibrateStrengthLeft,
					param.vibrateStrengthRight
				);

				Donya::Sound::Play( Music::BreakRock );
			}
		}
	};

	auto PlayerBody = [&]()
	{
		auto VS_Enemies = [&]()
		{
			Box player = pImpl->player.GetWorldSpaceBody( Player::BodyType::ToEnemy );
			Circle enemy{};

			size_t count = pImpl->enemies.GetEnemyCount();
			for ( size_t i = 0; i < count; ++i )
			{
				enemy = pImpl->enemies.GetWorldSpaceHitBox( i );
				if ( Circle::IsHitBox( enemy, player ) )
				{
					auto param = EnemyParameter::Get()->Open();
					float extrude = param.extrudeAmount;
					float knockback = param.knockbackAmount;
					pImpl->player.ReceiveDamage( knockback, extrude );

					break;
				}
			}
		};

		auto VS_Respawn = [&]()
		{
			Box player = pImpl->player.GetWorldSpaceBody( Player::BodyType::ToStage, /* isApplyInvisible = */false );

			auto pData = Config::GetDataOrNull( pImpl->currentStageNo );
			Box respawn = ( pData ) ? pData->respawn : Box::Nil();

			if ( Box::IsHitBox( player, respawn ) )
			{
				pImpl->hasRespawnPoint = true;
				Donya::Sound::Play( Music::TouchRespawnPoint );
			}
		};

		auto VS_Goal = [&]()
		{
			Box player = pImpl->player.GetWorldSpaceBody( Player::BodyType::ToStage, /* isApplyInvisible = */false );

			auto pData = Config::GetDataOrNull( pImpl->currentStageNo );
			Box goal = ( pData ) ? pData->goal : Box::Nil();

			if ( Box::IsHitBox( player, goal ) )
			{
				pImpl->didGoal = true;

				if ( !pImpl->pResultPerformer )
				{
					pImpl->UpdateHighScoreIfFasted( pImpl->currentStageNo );

					ResultPerformer::Create
					(
						&pImpl->pResultPerformer,
						pImpl->currentStageNo,
						pImpl->stageCount,
						pImpl->deathCount,
						pImpl->timers[pImpl->currentStageNo],
						pImpl->highScores[pImpl->currentStageNo]
					);
				}
			}
		};

		VS_Enemies();

		Box playerBody = pImpl->player.GetWorldSpaceBody( Player::BodyType::ToStage );

		size_t result = FindCollidingObject( pImpl->currentStageNo, /* vsDangerObj = */true, &playerBody, nullptr );
		if ( result != failedValue )
		{
			pImpl->player.SetMiss();
			pImpl->didMiss = true;
		}

		if ( !pImpl->hasRespawnPoint )
		{
			VS_Respawn();
		}

		if ( !pImpl->didMiss )
		{
			VS_Goal();
		}
	};

	PlayerAttack();

	PlayerBody();
}

Scene::Result SceneGame::ReturnResult()
{
	if ( Fader::Get().IsClosed() && pImpl->shouldChangeScene && pImpl->nextScene != Scene::Type::Null )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		// change.AddRequest( Scene::Request::FADE_THEN_ASSIGN );
		change.sceneType = pImpl->nextScene;
		return change;
	}
	// else

#if DEBUG_MODE
	if ( Donya::Keyboard::Trigger( VK_RETURN ) && Donya::Keyboard::Press( VK_RSHIFT ) )
	{
		Scene::Result change{};
		change.AddRequest( Scene::Request::ADD_SCENE, Scene::Request::REMOVE_ME );
		change.sceneType = Scene::Type::Title;
		return change;
	}
	// else
#endif // DEBUG_MODE

	Scene::Result noop{ Scene::Request::NONE, Scene::Type::Null };
	return noop;
}