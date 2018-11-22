#pragma once

namespace DemolisherWeapon {

	class CSprite
	{
	public:
		CSprite();
		~CSprite();

		void Init(const wchar_t* fileName);
		void Release();

		void Draw(const CVector2& pos, 
			const CVector4& color = CVector4::White(),
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			float layerDepth = 0.5f);

	private:
		DirectX::SpriteBatch* m_spriteBatch = nullptr;
		ID3D11ShaderResourceView* m_srv = nullptr;
	};

}
