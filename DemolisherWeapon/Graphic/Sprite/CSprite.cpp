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
		if (!TextureFactory::GetInstance().Load(fileName, &m_texdata)) {
			//失敗
			Release();
			return;
		}

		//SRV
		m_srv = m_texdata->textureView.Get();
		if (m_srv) { m_srv->AddRef(); }

		//GetGraphicsEngine().CreateDirectXTK12DescriptorNumber(m_cpuHandle, m_gpuHandle);
		//DirectX::CreateShaderResourceView(GetGraphicsEngine().GetD3D12Device(), m_texdata->d3d12texture.Get(), m_cpuHandle);

		//ファイルがDDSかどうかで乗算済みアルファ画像か判断
		if (m_texdata->isDDS) {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();
		}
		else {
			m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatchPMA();
		}

		//画像サイズの取得
		m_width = m_texdata->width;
		m_height = m_texdata->height;
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

#ifdef DW_DX12
		m_spriteBatch->Draw(m_gpuHandle, DirectX::GetTextureSize(m_texdata->d3d12texture.Get()), pos.vec, &m_sourceRectangle, color, rotation, DirectX::XMFLOAT2(pivot.x * m_width, pivot.y * m_height), DirectX::XMFLOAT2(scale.x, scale.y), effects, layerDepth);
#endif

#ifdef DW_DX11
		m_spriteBatch->Draw(m_srv, pos.vec, &m_sourceRectangle, color, rotation, DirectX::XMFLOAT2(pivot.x*m_width, pivot.y*m_height), DirectX::XMFLOAT2(scale.x, scale.y), effects, layerDepth);
#endif
	}

}