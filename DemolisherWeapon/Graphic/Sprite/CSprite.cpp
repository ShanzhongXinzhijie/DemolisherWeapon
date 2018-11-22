#include "DWstdafx.h"
#include "CSprite.h"

namespace DemolisherWeapon {

	CSprite::CSprite()
	{
		m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();
	}
	CSprite::~CSprite()
	{
		Release();
	}

	void CSprite::Init(const wchar_t* fileName) {
		DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), fileName, nullptr, &m_srv);
	}
	void CSprite::Release() {
		m_srv->Release();
	}

	void CSprite::Draw(
		const CVector2& pos,
		const CVector4& color,
		const CVector2& scale,
		const CVector2& pivot,
		float rotation,
		float layerDepth
	) {

		if (!m_srv) { return; }

		RECT sourceRectangle = { 0,0,200,200 };

		m_spriteBatch->Draw(m_srv, pos.vec, &sourceRectangle, color, rotation, DirectX::XMFLOAT2(pivot.x, pivot.y),	DirectX::XMFLOAT2(scale.x, scale.y), DirectX::SpriteEffects_None, layerDepth);
	}

}