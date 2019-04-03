#pragma once
#include "IRander.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {

class AmbientOcclusionRender : public IRander
{
public:
	AmbientOcclusionRender();
	~AmbientOcclusionRender();

	void Init(float texScale = 0.5f);
	void Release();

	void Render()override;

	//シェーダーリソースビューの取得
	ID3D11ShaderResourceView*& GetAmbientOcclusionSRV();
	ID3D11ShaderResourceView*& GetAmbientOcclusionBlurSRV();

	//有効無効の設定
	void SetEnable(bool enable) { m_enable = enable; }
	bool GetEnable()const { return m_enable; }

private:
	bool m_enable = true;

	UINT m_textureSizeX=0, m_textureSizeY=0;

	Shader m_cs;
	ID3D11Texture2D* m_ambientOcclusionTex = nullptr;
	ID3D11ShaderResourceView* m_ambientOcclusionSRV = nullptr;
	ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
	ID3D11Buffer* m_cb = nullptr;

	//定数バッファ
	struct SCSConstantBuffer {
		//フレームバッファ解像度
		unsigned int win_x;
		unsigned int win_y;

		//AO出力テクスチャ解像度
		unsigned int ao_x;
		unsigned int ao_y;		

		//ビュープロジェクション逆行列
		CMatrix mViewProjInv;

		//距離スケール
		float distanceScale;
	};

	GaussianBlurRender m_gaussBlur;
};

}