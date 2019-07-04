#pragma once

#include <filesystem>

namespace DemolisherWeapon {

	class CSprite
	{
	public:
		CSprite();
		~CSprite();

		void Release();

		//初期化
		void Init(const wchar_t* fileName);

		//描画する
		//※layerDepthの第三小数点以下は使わないほうがいいと思う
		//const CVector2& pos は0.0f～1.0fの範囲。{0.0f,0.0f}で画面左上
		void Draw(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);
		//const CVector2& pos をスクリーン座標で指定する版。右下の座標=画面解像度　
		void DrawScreenPos(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);

		//画像の描画する範囲を設定
		//0.0f～1.0fの範囲で指定
		void SetSourceRectangle(const float left, const float top, const float right, const float bottom) {
			m_sourceRectangle.left		= (LONG)(m_width  * left);
			m_sourceRectangle.right		= (LONG)(m_width  * right);
			m_sourceRectangle.top		= (LONG)(m_height * top);
			m_sourceRectangle.bottom	= (LONG)(m_height * bottom);
		}
		//画像の解像度で指定
		void SetSourceRectangleSpriteSize(const RECT& sourceRectangle){
			m_sourceRectangle = sourceRectangle;
		}

	private:
		DirectX::SpriteBatch* m_spriteBatch = nullptr;
		CVector2 m_screenSize;

		ID3D11ShaderResourceView* m_srv = nullptr;
		ID3D11Resource* m_tex = nullptr;

		UINT m_width, m_height;

		RECT m_sourceRectangle;
	};

}
