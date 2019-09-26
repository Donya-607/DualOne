#ifndef _INCLUDED_AUDIO_SYSTEM_H_
#define _INCLUDED_AUDIO_SYSTEM_H_

#include <memory>
#include <unordered_map>

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	namespace Studio
	{
		class System;
	}
}

namespace Donya
{
	class AudioSystem
	{
		// reference to https://books.google.co.jp/books?id=-2Z9DwAAQBAJ&pg=PA233&lpg=PA233&dq=FMOD+API+%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9&source=bl&ots=Lr5E9R80lv&sig=ACfU3U2qol2OMj9f0DF5vOof5vc6XrDbOw&hl=ja&sa=X&ved=2ahUKEwjzrInMn-HiAhVIWbwKHRemCB0Q6AEwAnoECAkQAQ#v=onepage&q=FMOD%20API%20%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9&f=false

	private:
		class Channels;
	private:
		FMOD::System			*pLowSystem;
		FMOD::Studio::System	*pSystem;

		std::unordered_map<size_t, FMOD::Sound *>	sounds;		// HACK:Is it better to shared_ptr ?
		// std::unordered_map<size_t, Channels *>		channels;	// HACK:Is it better to shared_ptr ?
		std::unordered_map<size_t, std::unique_ptr<Channels>>		channels;
	public:
		AudioSystem();
		~AudioSystem();
		AudioSystem( const AudioSystem & ) = delete;
		AudioSystem( const AudioSystem && ) = delete;
		AudioSystem & operator = ( const AudioSystem & ) = delete;
		AudioSystem & operator = ( const AudioSystem && ) = delete;
	public:
		/// <summary>
		/// Please call every frame.
		/// </summary>
		void Update( float elapsedTime );
	public:
		/// <summary>
		/// Please set relative-path or whole-path to fileName.<para></para>
		/// If load successed, returns unique handle of sound.<para></para>
		/// If load failed, returns NULL. <para></para>
		/// If fileName is already loaded, returns that loaded handle.
		/// </summary>
		size_t Load( std::string fileName, bool isEnableLoop );

		/// <summary>
		/// Play the sound identified by handle.<para></para>
		/// If failed play, or not found, returns false.
		/// </summary>
		bool Play( size_t soundHandle );

		/// <summary>
		/// Pause the sound identified by handle.<para></para>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed pause, or not found, returns false.
		/// </summary>
		bool Pause( size_t soundHandle, bool isEnableForAll = false );
		
		/// <summary>
		/// Resume the sound identified by handle.<para></para>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed pause, or not found, returns false.
		/// </summary>
		bool Resume( size_t soundHandle, bool isEnableForAll = false, bool fromTheBeginning = false );

		/// <summary>
		/// Stop the sound identified by handle.<para></para>
		/// If you want apply for all, set true to "isEnableForAll".<para></para>
		/// If failed stop, or not found, returns false.
		/// </summary>
		bool Stop( size_t soundHandle, bool isEnableForAll = false );

		/// <summary>
		/// Release the sound identified by handle.<para></para>
		/// If failed release, or not found, returns false.
		/// </summary>
		bool Release( size_t soundHandle );

		/// <summary>
		/// Release every sound.<para></para>
		/// If even one could not released, returns false.
		/// </summary>
		bool ReleaseAll();
	public:
		/// <summary>
		/// If failed count, returns -1.
		/// </summary>
		int GetNowPlayingSoundsCount();
	};
}

#endif // !_INCLUDED_AUDIO_SYSTEM_H_
