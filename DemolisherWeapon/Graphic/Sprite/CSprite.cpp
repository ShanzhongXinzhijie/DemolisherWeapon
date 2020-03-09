#include "DWstdafx.h"
#include "CSprite.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {

	CSprite::CSprite()
	{
		//m_screenSize.x = GetEngine().GetGraphicsEngine().GetFrameBuffer_W();
		//m_screenSize.y = GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
	}
	CSprite::~CSprite()
	{
		Release();
	}

	void CSprite::Init(const wchar_t* fileName) {
		Release();

		//ファクトリからテクスチャ読み込み
		const TextureFactory::TextueData* texdata = nullptr;
		if (!TextureFactory::GetInstance().Load(fileName, &m_tex, &m_srv, &texdata)) {
			//失敗
			Release();
			return;
		}
		//ファイルがDDSかどうかで乗算済みアルファ画像か判断
		if (texdata->isDDS) {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();
		}
		else {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatchPMA();
		}

		//画像サイズの取得
		m_width = texdata->width;
		m_height = texdata->height;
		m_sourceRectangle.top = 0;
		m_sourceRectangle.left = 0;
		m_sourceRectangle.bottom = m_height;
		m_sourceRectangle.right = m_width;
	}

	void CSprite::Init(ID3D11ShaderResourceView* srv, UINT width, UINT height, bool isPMA) {
		Release();

		m_srv = srv;
		m_srv->AddRef();

		if (!isPMA) {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();
		}
		else {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatchPMA();
		}

		//画像サイズの取得
		m_width = width;
		m_height = height;
		m_sourceRectangle.top = 0;
		m_sourceRectangle.left = 0;
		m_sourceRectangle.bottom = m_height;
		m_sourceRectangle.right = m_width;
	}

	void CSprite::Release() {
		if (m_tex) { m_tex->Release(); m_tex = nullptr; }
		if (m_srv) { m_srv->Release(); m_srv = nullptr; }
	}

	void CSprite::Draw(
		const CVector2& pos,
		const CVector2& scale,
		const CVector2& pivot,
		float rotation,
		const CVector4& color,
		DirectX::SpriteEffects effects,
		float layerDepth
	) {
		DrawScreenPos({ pos.x * GetGraphicsEngine().GetFrameBuffer_W(), pos.y * GetGraphicsEngine().GetFrameBuffer_H() }, scale, pivot, rotation, color, effects, layerDepth);
	}
	void CSprite::DrawScreenPos(
		const CVector2& pos,
		const CVector2& scale,
		const CVector2& pivot,
		float rotation,
		const CVector4& color,
		DirectX::SpriteEffects effects,
		float layerDepth
	) {

		if (!m_srv) { return; }

		layerDepth *= 0.999f; layerDepth += 0.001f;
		layerDepth -= GetEngine().GetGraphicsEngine().AddAndGetLayerDepthCnt();

		m_spriteBatch->Draw(m_srv, pos.vec, &m_sourceRectangle, color, rotation, DirectX::XMFLOAT2(pivot.x*m_width, pivot.y*m_height), DirectX::XMFLOAT2(scale.x, scale.y), effects, layerDepth);
	}

}