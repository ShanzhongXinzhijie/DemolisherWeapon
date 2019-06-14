#pragma once

namespace DemolisherWeapon {
	class CBillboard 
	{
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
				m_insModel->SetPos(pos);
				return;
			}
			m_model->SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			if (m_isIns) {
				m_insModel->SetRot(rot);
				return;
			}
			m_model->SetRot(rot);
		}
		void SetScale(const CVector3& scale) {
			if (m_isIns) {
				m_insModel->SetScale(scale);
				return;
			}
			m_model->SetScale(scale);
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			if (m_isIns) {
				return m_insModel->GetPos();
			}
			return m_model->GetPos();
		}
		const CQuaternion& GetRot() const {
			if (m_isIns) {
				return m_insModel->GetRot();
			}
			return m_model->GetRot();
		}
		const CVector3& GetScale() const {
			if (m_isIns) {
				return m_insModel->GetScale();
			}
			return m_model->GetScale();
		}
		void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}
		//�X�P�[���̍ő�l���擾
		float GetMaxScale()const {
			const CVector3& scale = GetScale();
			return max(max(scale.x, scale.y), scale.z);
		}		

		//�`�悷�邩�ݒ�
		void SetIsDraw(bool isdraw) {
			if (m_isIns) {
				m_insModel->SetIsDraw(isdraw);
			}
			else {
				m_model->SetIsDraw(isdraw);
			}
		}
		bool GetIsDraw()const {
			if (m_isIns) {
				return m_insModel->GetIsDraw();
			}
			else {
				return m_model->GetIsDraw();
			}
		}

		/// <summary>
		/// �C���X�^���V���O���f�����擾
		/// </summary>
		/// <returns>�C���X�^���V���O���f����?</returns>
		bool GetIsInstancing()const {
			return m_isIns;
		}

		//���f���̎擾
		GameObj::CSkinModelRender& GetModel() {
			if (m_isIns) {
				return m_insModel->GetInstancingModel()->GetModelRender();
			}
			else {
				return *m_model;
			}
		}
		//�C���X�^���V���O���f���̎擾
		GameObj::CInstancingModelRender& GetInstancingModel() {
#ifndef DW_MASTER
			if (!m_isIns) {
				OutputDebugStringA("�y�x���zCBillBoard::GetInstancingModel() �����̃r���{�[�h�̓C���X�^���V���O�`�悶��Ȃ���?\n");
			}
#endif
			return *m_insModel;
		}

	private:
		bool m_isInit = false;
		bool m_isIns = false;
		std::unique_ptr<GameObj::CSkinModelRender> m_model;
		std::unique_ptr<GameObj::CInstancingModelRender> m_insModel;

		Shader m_vsShader, m_vsZShader;
	};
}