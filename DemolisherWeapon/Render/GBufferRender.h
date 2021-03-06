#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

class SkinModel;

class GBufferRender :
	public IRander
{
public:
	enum EnGBuffer {
		enGBufferAlbedo,
		enGBufferNormal,
		enGBufferPosition,
		enGBufferVelocity,
		enGBufferVelocityPS,
		enGBufferLightParam,
		enGBufferNum,
	};

public:
	GBufferRender() = default;
	~GBufferRender() { Release(); }

	void Init();
	void Release();

	void Resize()override;
	
	ID3D11Texture2D* GetGBufferTex(EnGBuffer num) {
		return m_GBufferTex[num];
	}
	ID3D11ShaderResourceView*& GetGBufferSRV(EnGBuffer num) {
		return m_GBufferSRV[num];
	}
	ID3D11ShaderResourceView*& GetDepthStencilSRV() {
		return m_depthStencilSRV;
	}
	ID3D11DepthStencilView* GetDSV() { return m_depthStencilView; }

	void Render()override;
	void PostRender()override;

	void AddDrawModel(SkinModel* sm, int priority, bool reverse) {
		m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back( sm,reverse );
	};

private:
	std::list<std::pair<SkinModel*, bool>> m_drawModelList[DRAW_PRIORITY_MAX];

	ID3D11Texture2D*		m_GBufferTex[enGBufferNum] = { nullptr };	//GBufferテクスチャ
	ID3D11RenderTargetView* m_GBufferView[enGBufferNum] = { nullptr };	//GBufferビュー
	ID3D11ShaderResourceView* m_GBufferSRV[enGBufferNum] = { nullptr };	//GBufferSRV
	ID3D11Texture2D*		m_depthStencilTex = nullptr;				//デプスステンシルテクスチャ
	ID3D11DepthStencilView* m_depthStencilView = nullptr;				//デプスステンシルビュー
	ID3D11ShaderResourceView* m_depthStencilSRV = nullptr;				//デプスステンシルSRV

	//Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;//ブレンドステート	
};

}