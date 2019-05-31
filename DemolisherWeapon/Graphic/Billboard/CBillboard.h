#pragma once

namespace DemolisherWeapon {
//namespace GameObj{
	class CBillboard //: public IGameObject
	{
	public:
		CBillboard();
		~CBillboard();

		//void PostLoopUpdate()override;

	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="fileName">テクスチャ画像のファイルパス</param>
		/// <param name="instancingNum">インスタンシング描画数</param>
		void Init(std::experimental::filesystem::path fileName, int instancingNum = 1);
		//SRVから初期化
		void Init(ID3D11ShaderResourceView* srv, int instancingNum = 1, const wchar_t* identifiers = nullptr);

		//座標・回転・拡大の設定
		void SetPos(const CVector3& pos) {
			if (m_isIns) {
				m_insModel.SetPos(pos);
				return;
			}
			m_model.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			if (m_isIns) {
				m_insModel.SetRot(rot);
				return;
			}
			m_model.SetRot(rot);
		}
		void SetScale(const CVector3& scale) {
			if (m_isIns) {
				m_insModel.SetScale(scale);
				return;
			}
			m_model.SetScale(scale);
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//座標・回転・拡大の取得
		const CVector3& GetPos() const {
			if (m_isIns) {
				return m_insModel.GetPos();
			}
			return m_model.GetPos();
		}
		const CQuaternion& GetRot() const {
			if (m_isIns) {
				return m_insModel.GetRot();
			}
			return m_model.GetRot();
		}
		const CVector3& GetScale() const {
			if (m_isIns) {
				return m_insModel.GetScale();
			}
			return m_model.GetScale();
		}
		void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}

		//描画するか設定
		void SetIsDraw(bool isdraw) {
			if (m_isIns) {
				m_insModel.SetIsDraw(isdraw);
			}
			else {
				m_model.SetIsDraw(isdraw);
			}
		}

		/// <summary>
		/// インスタンシングモデルか取得
		/// </summary>
		/// <returns>インスタンシングモデルか?</returns>
		bool GetIsInstancing()const {
			return m_isIns;
		}

		//モデルの取得
		GameObj::CSkinModelRender& GetModel() {
			if (m_isIns) {
				return m_insModel.GetInstancingModel()->GetModelRender();
			}
			else {
				return m_model;
			}
		}
		//インスタンシングモデルの取得
		GameObj::CInstancingModelRender& GetInstancingModel() {
#ifndef DW_MASTER
			if (!m_isIns) {
				OutputDebugStringA("【警告】CBillBoard::GetInstancingModel() ※このビルボードはインスタンシング描画じゃないよ?\n");
			}
#endif
			return m_insModel;
		}

	private:
		bool m_isInit = false;
		bool m_isIns = false;
		GameObj::CSkinModelRender m_model;
		GameObj::CInstancingModelRender m_insModel;
	};
//}
}