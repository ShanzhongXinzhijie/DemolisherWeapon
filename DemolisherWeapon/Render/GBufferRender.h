#pragma once
#include "IRander.h"
#include "Graphic/Model/SkinModel.h"

namespace DemolisherWeapon {

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
		enGBufferNum,
	};

public:
	GBufferRender();
	~GBufferRender();

	void Init();
	void Release();
	
	ID3D11Texture2D* GetGBufferTex(EnGBuffer num) {
		return m_GBufferTex[num];
	}
	ID3D11ShaderResourceView*& GetGBufferSRV(EnGBuffer num) {
		return m_GBufferSRV[num];
	}
	ID3D11ShaderResourceView*& GetDepthStencilSRV() {
		return m_depthStencilSRV;
	}

	void Render()override;

	void AddDrawModel(SkinModel* sm) {
		m_drawModelList.emplace_back(sm);
	};

private:

	std::list<SkinModel*> m_drawModelList;

	ID3D11Texture2D*		m_GBufferTex[enGBufferNum] = { nullptr };	//GBufferテクスチャ
	ID3D11RenderTargetView* m_GBufferView[enGBufferNum] = { nullptr };	//GBufferビュー
	ID3D11ShaderResourceView* m_GBufferSRV[enGBufferNum] = { nullptr };	//GBufferSRV
	ID3D11Texture2D*		m_depthStencilTex = nullptr;				//デプスステンシルテクスチャ
	ID3D11DepthStencilView* m_depthStencilView = nullptr;				//デプスステンシルビュー
	ID3D11ShaderResourceView* m_depthStencilSRV = nullptr;				//デプスステンシルSRV
};

}