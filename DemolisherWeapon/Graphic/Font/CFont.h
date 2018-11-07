#pragma once

namespace DemolisherWeapon {

	class CFont
	{
	public:
		CFont();
		~CFont();

		//�t�H���g�����[�h����

		//�t�H���g��ύX����
		void ChangeFont(DirectX::SpriteFont* font) {
			m_spriteFont = font;
		}

		//�`�悷��
		//const CVector2& pos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
		void Draw(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			float layerDepth = 0.5f
		);
		//const CVector2& pos ���X�N���[�����W�Ŏw�肷��ŁB�E���̍��W=��ʉ𑜓x�@
		void DrawScreenPos(
			wchar_t const* text,
			const CVector2& pos,
			const CVector4& color = CVector4::White(),
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			float layerDepth = 0.5f
		);

	private:
		DirectX::SpriteBatch* m_spriteBatch = nullptr;
		DirectX::SpriteFont* m_spriteFont = nullptr;
		CVector2 m_screenSize;
	};

}