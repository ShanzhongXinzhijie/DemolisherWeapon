#pragma once

namespace DemolisherWeapon {

	class CSprite
	{
	public:
		CSprite();
		~CSprite();

		void Release();

		//������
		void Init(const wchar_t* fileName);

		//�`�悷��
		//const CVector2& pos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
		void Draw(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);
		//const CVector2& pos ���X�N���[�����W�Ŏw�肷��ŁB�E���̍��W=��ʉ𑜓x�@
		void DrawScreenPos(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);

		//�摜�̕`�悷��͈͂�ݒ�
		//0.0f�`1.0f�͈̔͂Ŏw��
		void SetSourceRectangle(const float left, const float top, const float right, const float bottom) {
			m_sourceRectangle.left		= (LONG)(m_width  * left);
			m_sourceRectangle.right		= (LONG)(m_width  * right);
			m_sourceRectangle.top		= (LONG)(m_height * top);
			m_sourceRectangle.bottom	= (LONG)(m_height * bottom);
		}
		//�摜�̉𑜓x�Ŏw��
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
