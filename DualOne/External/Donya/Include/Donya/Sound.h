#ifndef _INCLUDED_SOUND_H_
#define _INCLUDED_SOUND_H_

#include <string>

namespace Donya
{
	/// <summary>
	/// This namespace is facade-pattern.<para></para>
	/// For users to use AudioSystem from anywhere.
	/// </summary>
	namespace Sound
	{
		/// <summary>
		/// Doing initialize and load sounds.
		/// </summary>
		void Init();
		/// <summary>
		/// Doing release sounds and uninitialize.
		/// </summary>
		void Uninit();

		/// <summary>
		/// Please call every frame.
		/// </summary>
		void Update( float elapsedTime );

		/// <summary>
		/// You can register the directory used by Donya::Sound::Load().<para></para>
		/// registered name are used just combine before Load()'s file name.<para></para>
		/// for exanmple:<para></para>
		/// RegisterDirectoryOfLoadFile( "./Data/Sounds/" );<para></para>
		/// Load( "BGM.wav", /*isEnableLoop = */ true ); // Loading file name is "./Data/Sounds/BGM.wav".<para></para>
		/// Load( "SE.wav", /*isEnableLoop = */ false ); // Loading file name is "./Data/Sounds/SE.wav".<para></para>
		/// </summary>
		void RegisterDirectoryOfLoadFile( const char *fileDirectory );
		/// <summary>
		/// The soundIdentifier is became identifier of the sound of another sound function.<para></para>
		/// Please set relative-path or whole-path to fileName.<para></para>
		/// If already registered the directory name, I combine the directory name before fileName.<para></para>
		/// If load successed or already loaded, returns true.
		/// </summary>
		bool Load( int soundIdentifier, std::string fileName, bool isEnableLoop );

		/// <summary>
		/// If failed play sound, or the identifier is incorrect, returns false.
		/// </summary>
		bool Play( int soundIdentifier );
		// TODO:I want user can specify play mode(ex:loop).

		/// <summary>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed pause sound, or the identifier is incorrect, returns false.
		/// </summary>
		bool Pause( int soundIdentifier, bool isEnableForAll = false );

		/// <summary>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed resume sound, or the identifier is incorrect, returns false.
		/// </summary>
		bool Resume( int soundIdentifier, bool isEnableForAll = false, bool fromTheBeginning = false );

		/// <summary>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed stop sound, or the identifier is incorrect, returns false.
		/// </summary>
		bool Stop( int soundIdentifier, bool isEnableForAll = false );

		/// <summary>
		/// If failed count, returns -1.
		/// </summary>
		int  GetNowPlayingSoundsCount();
	}
}

#endif // !_INCLUDED_SOUND_H_
