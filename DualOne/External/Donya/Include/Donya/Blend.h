#ifndef INCLUDED_BLEND_H_
#define INCLUDED_BLEND_H_

#include <array>
#include <D3D11.h>
#include <memory>
#include <wrl.h>

namespace Donya
{
	namespace Blend
	{
		enum class Mode : int
		{
			NO_BLEND = 0,
			ALPHA,			//																( RGB : dest * ( 1 - srcA ) + ( src * srcA ),	A : destA * ( 1 - srcA ) + srcA )
			ADD,			// addition dest by src * src.alpha, dest alpha is immutable.	( RGB : dest				+ ( src * srcA ),	A : destA )
			SUB,			// subtract dest by src * src.alpha, dest alpha is immutable.	( RGB : dest				- ( src * srcA ),	A : destA )
			MUL,			// multiply each component.										( RGB : src * dest,								A : srcA * destA )
			SCREEN,			//																( RGB : dest * ( 1 - src  ) + ( src * srcA ),	A : destA * ( 1 - srcA ) + srcA )
			LIGHTEN,		//																( RGB : max( src, dest ),						A : max( srcA, destA ) )
			DARKEN,			//																( RGB : min( src, dest ),						A : min( srcA, destA ) )

			END_OF_ENUMERATOR
		};

		class State
		{
		private:
			Mode mode;
			Microsoft::WRL::ComPtr<ID3D11BlendState> d3dBlendState;
		public:
			/// <summary>
			/// do 'CreateBlendState'.<para></para>
			/// if you want another setting, set to third argument.
			/// in that case, I ignore the 'blendMode' parameter.
			/// </summary>
			State( ID3D11Device *pd3dDevice, Mode blendMode, D3D11_BLEND_DESC *extraBlendDesc = nullptr );
			~State() {}
		public:
			Mode GetKind() const
			{
				return mode;
			}
			ID3D11BlendState *Get()
			{
				return d3dBlendState.Get();
			}
			ID3D11BlendState **GetAddressOf()
			{
				return d3dBlendState.GetAddressOf();
			}
		};

		class Storage
		{
		private:
			static std::unique_ptr<Storage> instance;
		public:
			static Storage *GetInstance()
			{
				if ( !instance )
				{
					instance.reset( new Storage() );
				}

				return instance.get();
			}
		private:
			std::array<std::unique_ptr<State>, static_cast<size_t>( Blend::Mode::END_OF_ENUMERATOR )> pStates;
		private:
			Storage() : pStates()
			{}
		public:
			~Storage() {}
		public:

		#pragma region CreateEachBlendStates

			bool CreateBlendStateNoBlend();		// returns true if created.
			bool CreateBlendStateAlpha();		// returns true if created.
			bool CreateBlendStateAdd();			// returns true if created.
			bool CreateBlendStateSub();			// returns true if created.
			bool CreateBlendStateMul();			// returns true if created.
			bool CreateBlendStateScreen();		// returns true if created.
			bool CreateBlendStateLighter();		// returns true if created.
			bool CreateBlendStateDarker();		// returns true if created.
			// bool CreateBlendStateInverse	();	// returns true if created.
			bool CreateAllBlendState();			// returns true if all created.

		#pragma endregion

			void SetBlendMode( Blend::Mode blendMode );
		};

		void Init();
		void Set( Blend::Mode blendMode );
	}
}

#endif // INCLUDED_BLEND_H_