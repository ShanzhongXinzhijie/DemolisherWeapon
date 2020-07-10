#pragma once

namespace DemolisherWeapon {

	class FontBank {
	public:
		DirectX::SpriteFont* Load(const wchar_t* path) {
			int key = Util::MakeHash(path);
			if (m_fontmap.count(key) == 0) {
#ifndef DW_DX12_TEMPORARY
				m_fontmap.emplace(key, std::make_unique<DirectX::SpriteFont>(GetEngine().GetGraphicsEngine().GetD3DDevice(), path));
#endif
			}
			return m_fontmap[key].get();
		}
	private:
		std::unordered_map<int, std::unique_ptr<DirectX::SpriteFont>> m_fontmap;
	};

	class CFont
	{
	public:
		//デフォルトのフォントをロード
		static void LoadDefaultFont(const wchar_t* path) {
			m_s_defaultFont = m_s_fontBank.Load(path);
		}		
		//デフォルトのフォントを変更する
		//nullで初期設定に戻る
		static void ChangeDefaultFont(DirectX::SpriteFont* font) {
			m_s_defaultFont = font;
		}
	public:
		CFont();
		CFont(const wchar_t* path);
		~CFont();

		//フォントをロードする
		void LoadFont(const wchar_t* path) {
			m_spriteFont = m_s_fontBank.Load(path);
		}
		//フォントを変更する
		void ChangeFont(DirectX::SpriteFont* font) {
			m_spriteFont = font;
		}
		//初期設定のシステムフォントを使用する
		void UseSystemFont() {
			m_spriteFont = GetEngine().GetGraphicsEngine().GetSpriteFont();
		}
		//フォントを取得
		DirectX::SpriteFont* GetFont()const {
			return m_spriteFont;
		}

		//描画する
		//※layerDepthの第三小数点以下は使わないほうがいいと思う
		//layerDepthは0.0が最前面(たぶん)
		//const CVector2& pos は0.0f～1.0fの範囲。{0.0f,0.0f}で画面左上
		void Draw(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f
		)const;
		//const CVector2& pos をスクリーン座標で指定する版。右下の座標=画面解像度　
		void DrawScreenPos(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f
		)const;

	private:
		void initialize();

		DirectX::SpriteBatch* m_spriteBatch = nullptr;
		DirectX::SpriteFont* m_spriteFont = nullptr;
		//CVector2 m_screenSize;

		static FontBank m_s_fontBank;
		static DirectX::SpriteFont* m_s_defaultFont;
	};

}