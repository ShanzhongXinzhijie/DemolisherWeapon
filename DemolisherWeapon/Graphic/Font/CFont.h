#pragma once

namespace DemolisherWeapon {

	class FontBank {
	public:
		DirectX::SpriteFont* Load(const wchar_t* path) {
			int key = Util::MakeHash(path);
			if (m_fontmap.count(key) == 0) {
				m_fontmap.emplace(key, std::make_unique<DirectX::SpriteFont>(GetEngine().GetGraphicsEngine().GetD3DDevice(), path));
			}
			return m_fontmap[key].get();
		}
	private:
		std::unordered_map<int, std::unique_ptr<DirectX::SpriteFont>> m_fontmap;
	};

	class CFont
	{
	public:
		//�f�t�H���g�̃t�H���g�����[�h
		static void LoadDefaultFont(const wchar_t* path) {
			m_s_defaultFont = m_s_fontBank.Load(path);
		}		
		//�f�t�H���g�̃t�H���g��ύX����
		//null�ŏ����ݒ�ɖ߂�
		static void ChangeDefaultFont(DirectX::SpriteFont* font) {
			m_s_defaultFont = font;
		}
	public:
		CFont();
		CFont(const wchar_t* path);
		~CFont();

		//�t�H���g�����[�h����
		void LoadFont(const wchar_t* path) {
			m_spriteFont = m_s_fontBank.Load(path);
		}
		//�t�H���g��ύX����
		void ChangeFont(DirectX::SpriteFont* font) {
			m_spriteFont = font;
		}
		//�����ݒ�̃V�X�e���t�H���g���g�p����
		void UseSystemFont() {
			m_spriteFont = GetEngine().GetGraphicsEngine().GetSpriteFont();
		}
		//�t�H���g���擾
		DirectX::SpriteFont* GetFont()const {
			return m_spriteFont;
		}

		//�`�悷��
		//��layerDepth�̑�O�����_�ȉ��͎g��Ȃ��ق��������Ǝv��
		//const CVector2& pos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
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
		//const CVector2& pos ���X�N���[�����W�Ŏw�肷��ŁB�E���̍��W=��ʉ𑜓x�@
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