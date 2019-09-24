#include <locale.h>
#include <time.h>
#include <windows.h>

#include "Common.h"
#include "Constant.h"
#include "Donya.h"
#include "Framework.h"
#include "Icon.h"
#include "Sound.h"

INT WINAPI wWinMain( _In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPWSTR cmdLine, _In_ INT cmdShow )
{
#if defined( DEBUG ) | defined( _DEBUG )
	// reference:https://docs.microsoft.com/ja-jp/visualstudio/debugger/crt-debug-heap-details?view=vs-2015
	_CrtSetDbgFlag
	(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
		// | _CRTDBG_CHECK_ALWAYS_DF
	);
	// When memory leak detected, if you assign the output number to _crtBreakAlloc,
	// program will be stop in that memory allocate place.
	// _crtBreakAlloc = ;
#endif

	setlocale( LC_ALL, "JPN" );

	srand( scast<unsigned int>( time( NULL ) ) );

	std::string title{ "��������" };
	Donya::Init( cmdShow, Common::ScreenWidth(), Common::ScreenHeight(), title.c_str() );

	Donya::SetWindowIcon( instance, IDI_ICON );

	Framework framework{};
	framework.Init();

	while ( Donya::MessageLoop() )
	{
		Donya::ClearViews();

		Donya::SystemUpdate();
		framework.Update( Donya::GetElapsedTime() );

		framework.Draw( Donya::GetElapsedTime() );
		Donya::Present( 1 );
	}

	framework.Uninit();

	auto returnValue = Donya::Uninit();

	return returnValue;
}