#pragma once

namespace DemolisherWeapon {
	class CBillboard : public IGameObject
	{
	public:
		CBillboard();
		~CBillboard();

		void Update()override;

	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="fileName">�e�N�X�`���摜�̃t�@�C���p�X</param>
		void Init(std::experimental::filesystem::path fileName);

		//���W�E��]�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_model.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			m_rot = rot;
		}
		void SetScale(const CVector3& scale) {
			m_model.SetScale(scale);
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			return m_model.GetPos();
		}
		const CQuaternion& GetRot() const {
			return m_rot;
		}
		const CVector3& GetScale() const {
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
		GameObj::CSkinModelRender m_model;
		CQuaternion m_rot;
	};
}