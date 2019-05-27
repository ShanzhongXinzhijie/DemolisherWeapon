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
		/// ������
		/// </summary>
		/// <param name="fileName">�e�N�X�`���摜�̃t�@�C���p�X</param>
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		void Init(std::experimental::filesystem::path fileName, int instancingNum = 1);
		//SRV���珉����
		void Init(ID3D11ShaderResourceView* srv, int instancingNum = 1, const wchar_t* identifiers = nullptr);

		//���W�E��]�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			if (m_isIns) {
				m_insModel.SetPos(pos);
				return;
			}
			m_model.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			m_rot = rot;
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
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			if (m_isIns) {
				return m_insModel.GetPos();
			}
			return m_model.GetPos();
		}
		const CQuaternion& GetRot() const {
			return m_rot;
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

		//���f���̎擾
		GameObj::CSkinModelRender& GetModel() {
			return m_model;
		}
		//�C���X�^���V���O��
		GameObj::CInstancingModelRender& GetInstancingModel() {
			return m_insModel;
		}

		/// <summary>
		/// �r���{�[�h�N�H�[�^�j�I�����擾
		/// </summary>
		/// <returns></returns>
		static CQuaternion GetBillboardQuaternion();

		/// <summary>
		/// �r���{�[�h�s����擾
		/// </summary>
		/// <returns></returns>
		static CMatrix GetBillboardMatrix();

	private:
		bool m_isInit = false;
		bool m_isIns = false;
		GameObj::CSkinModelRender m_model;
		GameObj::CInstancingModelRender m_insModel;
		CQuaternion m_rot;
	};
//}
}