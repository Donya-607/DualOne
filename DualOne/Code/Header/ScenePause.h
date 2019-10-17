#pragma once

#include "Donya/GamepadXInput.h"
#include "Donya/Serializer.h"
#include "Donya/UseImgui.h"
#include "Donya/Vector.h"

#include "Scene.h"

class ScenePause : public Scene
{
private:
	enum Choice
	{
		Nil = -1,
		BackToTitle = 0,
		Resume,
		ReTry,
	};
private:
	Choice			choice;

	Donya::Vector2	uiPausePos;
	Donya::Vector2	uiRetryPos;
	Donya::Vector2	uiResumePos;
	Donya::Vector2	uiBackPos;
	Donya::Vector2	uiArrowPosL;
	Donya::Vector2	uiArrowPosR;
	
	Donya::XInput	controller;
public:
	ScenePause();
	~ScenePause();
private:
	friend class cereal::access;
	template<class Archive>
	void serialize( Archive &archive, const std::uint32_t version )
	{
		archive
		(
			CEREAL_NVP( uiPausePos ),
			CEREAL_NVP( uiRetryPos ),
			CEREAL_NVP( uiResumePos ),
			CEREAL_NVP( uiBackPos ),
			CEREAL_NVP( uiArrowPosL ),
			CEREAL_NVP( uiArrowPosR )
		);

		if ( 1 <= version )
		{
			// archive( CEREAL_NVP() );
		}
	}
	static constexpr const char *SERIAL_ID = "Pause";
public:
	void	Init();
	void	Uninit();

	Result	Update( float elapsedTime );

	void	Draw( float elapsedTime );
private:
	void	UpdateChooseItem();
	bool	IsDecisionTriggered() const;

	bool	ShouldUseFade( Choice choice ) const;
	void	StartFade();

	Result	ReturnResult();
public:
	void LoadParameter( bool isBinary = true );

#if USE_IMGUI

	void SaveParameter();

	void UseImGui();

#endif // USE_IMGUI
};
