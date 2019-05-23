#pragma once

namespace DemolisherWeapon {
	class CImposter
	{
	public:
		//初期化
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);
		
		//座標・回転・拡大の設定
		void SetPos(const CVector3& pos) {
			m_billboard.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			m_billboard.SetRot(rot);
		}
		void SetScale(const CVector3& scale) {
			m_billboard.SetScale(scale*(m_imposterMaxSize*2.0f));
			m_scale = scale;
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//座標・回転・拡大の取得
		const CVector3& GetPos() const {
			return m_billboard.GetPos();
		}
		const CQuaternion& GetRot() const {
			return m_billboard.GetRot();
		}
		const CVector3& GetScale() const {
			return m_scale;
		}
		void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}

	private:
		//初期家内初期化
		void Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum);
		//インポスタテクスチャの作成
		void RenderImposter(SkinModel& model);

	private:
		enum EnGBuffer {
			enGBufferAlbedo,
			enGBufferNormal,
			enGBufferLightParam,
			enGBufferNum,
		};
		UINT m_gbufferSizeX = 0, m_gbufferSizeY = 0;
		UINT m_partNumX = 0, m_partNumY = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_GBufferTex[enGBufferNum];	//GBufferテクスチャ
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_GBufferView[enGBufferNum];//GBufferビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_GBufferSRV[enGBufferNum];	//GBufferSRV
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_depthStencilTex;			//デプスステンシルテクスチャ
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_depthStencilView;			//デプスステンシルビュー

		SkinModelEffectShader m_imposterPS;
		float m_imposterMaxSize;
		// CVector2 m_imposterMaxSize;
		// アス比
		// Z-UPとY-UP

		//ビルボード
		GameObj::CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		CVector3 m_scale;
	};
}