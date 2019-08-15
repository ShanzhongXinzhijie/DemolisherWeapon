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
		enGBufferLightParam,
		enGbufferTranslucent,
		enGBufferNum,
	};

public:
	GBufferRender() = default;
	~GBufferRender() { Release(); }

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
	ID3D11DepthStencilView* GetDSV() { return m_depthStencilView; }

	void Render()override;
	void PostRender()override;

	void AddDrawModel(SkinModel* sm, int priority) {
		m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX-1)].emplace_back(sm);
	};

private:
	std::list<SkinModel*> m_drawModelList[DRAW_PRIORITY_MAX];

	ID3D11Texture2D*		m_GBufferTex[enGBufferNum] = { nullptr };	//GBuffer�e�N�X�`��
	ID3D11RenderTargetView* m_GBufferView[enGBufferNum] = { nullptr };	//GBuffer�r���[
	ID3D11ShaderResourceView* m_GBufferSRV[enGBufferNum] = { nullptr };	//GBufferSRV
	ID3D11Texture2D*		m_depthStencilTex = nullptr;				//�f�v�X�X�e���V���e�N�X�`��
	ID3D11DepthStencilView* m_depthStencilView = nullptr;				//�f�v�X�X�e���V���r���[
	ID3D11ShaderResourceView* m_depthStencilSRV = nullptr;				//�f�v�X�X�e���V��SRV

	//Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;//�u�����h�X�e�[�g	
};

}