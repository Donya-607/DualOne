#ifndef INCLUDED_GEOMETRIC_PRIMITIVE_H_
#define INCLUDED_GEOMETRIC_PRIMITIVE_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

namespace Donya
{
	namespace Geometric
	{
		// TODO:Prevent excessive initialize.

		class Base
		{
		public:
			struct Vertex
			{
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT3 normal;
			};
			struct ConstantBuffer
			{
				DirectX::XMFLOAT4X4	worldViewProjection;
				DirectX::XMFLOAT4X4	world;
				DirectX::XMFLOAT4	lightDirection;
				DirectX::XMFLOAT4	materialColor;
			};
		protected:
		#define	COM_PTR Microsoft::WRL::ComPtr
			mutable COM_PTR<ID3D11Buffer>				iVertexBuffer;
			mutable COM_PTR<ID3D11Buffer>				iIndexBuffer;
			mutable COM_PTR<ID3D11Buffer>				iConstantBuffer;
			mutable COM_PTR<ID3D11InputLayout>			iInputLayout;
			mutable COM_PTR<ID3D11VertexShader>			iVertexShader;
			mutable COM_PTR<ID3D11PixelShader>			iPixelShader;
			mutable COM_PTR<ID3D11RasterizerState>		iRasterizerStateWire;
			mutable COM_PTR<ID3D11RasterizerState>		iRasterizerStateSurface;
			mutable COM_PTR<ID3D11DepthStencilState>	iDepthStencilState;
		#undef	COM_PTR
			size_t indicesCount;
		public:
			Base();
			virtual ~Base() = default;
		public:
			virtual void Init() = 0;
			virtual void Uninit() = 0;

			virtual void Render
			(
				const DirectX::XMFLOAT4X4	&matWorldViewProjection,
				const DirectX::XMFLOAT4X4	&matWorld,
				const DirectX::XMFLOAT4		&lightDirection,
				const DirectX::XMFLOAT4		&materialColor,
				bool isEnableFill = true
			) const = 0;
		};

		class Cube : public Base
		{
		public:
			Cube();
			~Cube();
		public:
			void Init() override;
			void Uninit() override;

			void Render
			(
				const DirectX::XMFLOAT4X4	&matWorldViewProjection,
				const DirectX::XMFLOAT4X4	&matWorld,
				const DirectX::XMFLOAT4		&lightDirection,
				const DirectX::XMFLOAT4		&materialColor,
				bool isEnableFill = true
			) const override;
		};

		class Sphere : public Base
		{
		private:
			const size_t HORIZONTAL_SLICE{};
			const size_t VERTICAL_SLICE{};
		public:
			Sphere( size_t horizontalSliceCount = 12U, size_t verticalSliceCount = 6U );
			~Sphere();
		public:
			void Init() override;
			void Uninit() override;

			void Render
			(
				const DirectX::XMFLOAT4X4	&matWorldViewProjection,
				const DirectX::XMFLOAT4X4	&matWorld,
				const DirectX::XMFLOAT4		&lightDirection,
				const DirectX::XMFLOAT4		&materialColor,
				bool isEnableFill = true
			) const override;
		};

		/// <summary>
		/// The board of duplex printed.
		/// </summary>
		class TextureBoard : public Base
		{
		public:
			// TODO:Support drawing part of texture.
			struct Vertex : public Base::Vertex
			{
				DirectX::XMFLOAT2 texCoord;
				// DirectX::XMFLOAT4 texCoordTransform;
			};
		private:
			const std::wstring FILE_PATH{};	// Contain file-directory + file-name.
			mutable D3D11_TEXTURE2D_DESC textureDesc;
			mutable Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	iSRV;
			mutable Microsoft::WRL::ComPtr<ID3D11SamplerState>			iSampler;
		public:
			TextureBoard( std::wstring filePath );
			~TextureBoard();
		public:
			void Init() override;
			void Uninit() override;

			void Render
			(
				const DirectX::XMFLOAT4X4	&matWorldViewProjection,
				const DirectX::XMFLOAT4X4	&matWorld,
				const DirectX::XMFLOAT4		&lightDirection,
				const DirectX::XMFLOAT4		&materialColor,
				bool isEnableFill = true
			) const override;
		};
	}
}

#endif // !INCLUDED_GEOMETRIC_PRIMITIVE_H_
