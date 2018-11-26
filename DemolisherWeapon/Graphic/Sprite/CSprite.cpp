#include "DWstdafx.h"
#include "CSprite.h"

namespace DemolisherWeapon {

	CSprite::CSprite()
	{
		m_spriteBatch = GetEngine().GetGraphicsEngine().GetSpriteBatch();

		m_screenSize.x = GetEngine().GetGraphicsEngine().GetFrameBuffer_W();
		m_screenSize.y = GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
	}
	CSprite::~CSprite()
	{
		Release();
	}

	void CSprite::Init(const wchar_t* fileName) {
		Release();

		HRESULT hr = DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), fileName, &m_tex, &m_srv);
		if (FAILED(hr)) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "CSprite::Init()の画像読み込みに失敗。\nファイルパスあってますか？\n%ls\n", fileName);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			Release(); 
			return;
		}

		//画像サイズの取得
		D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		m_tex->GetType(&resType);
		switch (resType){
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				ID3D11Texture2D* tex2d = static_cast<ID3D11Texture2D*>(m_tex);
				D3D11_TEXTURE2D_DESC desc;
				tex2d->GetDesc(&desc);

				//画像サイズの取得
				m_width = desc.Width;
				m_height = desc.Height;
				m_sourceRectangle.top = 0;
				m_sourceRectangle.left = 0;
				m_sourceRectangle.bottom = desc.Height;
				m_sourceRectangle.right = desc.Width;
			}
			break;
		default:
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "CSprite::Init()「なんかちがう」\n%ls\n", fileName);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			Release();
			return;

			break;
		}
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
		DrawScreenPos({ pos.x * m_screenSize.x, pos.y * m_screenSize.y }, scale, pivot, rotation, color, effects, layerDepth);
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

		m_spriteBatch->Draw(m_srv, pos.vec, &m_sourceRectangle, color, rotation, DirectX::XMFLOAT2(pivot.x*m_width, pivot.y*m_height), DirectX::XMFLOAT2(scale.x, scale.y), effects, layerDepth);
	}

}