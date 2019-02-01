#include "DWstdafx.h"
#include "CFont.h"

namespace DemolisherWeapon {

	CFont::CFont()
	{
		m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();
		m_spriteFont = GetEngine().GetGraphicsEngine().GetSpriteFont();

		m_screenSize.x = GetEngine().GetGraphicsEngine().GetFrameBuffer_W();
		m_screenSize.y = GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
	}

	CFont::~CFont()
	{
	}

	void CFont::Draw(
		wchar_t const* text,
		const CVector2& pos,
		const CVector4& color,
		const CVector2& scale,
		const CVector2& pivot,
		float rotation,
		DirectX::SpriteEffects effects,
		float layerDepth
	) {
		DrawScreenPos(text, { pos.x * m_screenSize.x, pos.y * m_screenSize.y }, color, scale, pivot, rotation, effects, layerDepth);
	}

	void CFont::DrawScreenPos(
		wchar_t const* text,
		const CVector2& pos,
		const CVector4& color,
		const CVector2& scale,
		const CVector2& pivot,
		float rotation,
		DirectX::SpriteEffects effects,
		float layerDepth
	) {
		if (text == nullptr) {
			return;
		}		

		layerDepth *= 0.999f; layerDepth += 0.001f;
		layerDepth -= GetEngine().GetGraphicsEngine().AddAndGetLayerDepthCnt();

		m_spriteFont->DrawString(
			m_spriteBatch,
			text,
			pos.vec,
			color,
			rotation,
			DirectX::XMFLOAT2(pivot.x, pivot.y),
			DirectX::XMFLOAT2(scale.x, scale.y),
			effects,
			layerDepth
		);
	}

}