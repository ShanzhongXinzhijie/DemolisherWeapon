#pragma once

namespace DemolisherWeapon {

	class CFont
	{
	public:
		CFont();
		~CFont();

		//フォントをロードする

		//フォントを変更する
		void ChangeFont(DirectX::SpriteFont* font) {
			m_spriteFont = font;
		}

		//描画する
		//const CVector2& pos は0.0f～1.0fの範囲。{0.0f,0.0f}で画面左上
		void Draw(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			float rotation = 0.0f,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float layerDepth = 0.5f
		);
		//const CVector2& pos をスクリーン座標で指定する版。右下の座標=画面解像度　
		void DrawScreenPos(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			float rotation = 0.0f,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float layerDepth = 0.5f
		);

	private:
		DirectX::SpriteBatch* m_spriteBatch = nullptr;
		DirectX::SpriteFont* m_spriteFont = nullptr;
		CVector2 m_screenSize;
	};

}