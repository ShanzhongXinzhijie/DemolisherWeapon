#pragma once

namespace DemolisherWeapon {
//namespace GameObj{
	class CBillboard //: public IGameObject
	{
	public:
		//�C���X�^���V���O�ɂ�����e�C���X�^���X��SRT�s���ۑ����邽�߂̃N���X
		class InstancingSRTRecorder : public GameObj::InstancingModel::IInstancesData {
		private:
			void Reset(int instancingMaxNum);
		public:
			InstancingSRTRecorder(int instancingMaxNum);
			void PreDrawUpdate()override {}
			void PostLoopPostUpdate()override {}
			void AddDrawInstance(int instanceNum, const CMatrix& SRTMatrix)override;
		public:
			//�C���X�^���X�ő吔��ݒ�
			void SetInstanceMax(int instanceMax);
			//SRT�s��̎擾
			const std::unique_ptr<CMatrix[]>& GetSRTMatrix()const { return m_SRTMatrix; }
		private:
			int m_instanceMax = 0;
			std::unique_ptr<CMatrix[]>	m_SRTMatrix;
		};

		/// <summary>
		/// �V���h�E�}�b�v�`�掞�Ɏ��s���鏈��
		/// </summary>
		class ShodowWorldMatrixCalcer : public ShadowMapRender::IPrePost {
		public:
			ShodowWorldMatrixCalcer(CBillboard* model);
			void PreDraw()override;
			void PreModelDraw()override;
			void PostDraw()override;
		private:
			//float m_depthBias = 0.0f;
			CMatrix	m_worldMatrix;
			SkinModel* m_ptrModel = nullptr;
			CBillboard* m_ptrBillboard = nullptr;
		};
		//�C���X�^���V���O�p
		class ShodowWorldMatrixCalcerInstancing : public ShadowMapRender::IPrePost {
		public:
			ShodowWorldMatrixCalcerInstancing(CBillboard* model, InstancingSRTRecorder* insSRT);
			void PreDraw()override;
			void PreModelDraw()override;
			void PostDraw()override;
		private:
			int m_instancesNum = 0;
			std::unique_ptr<CMatrix[]>	m_worldMatrix;
			GameObj::InstancingModel* m_ptrModel = nullptr;
			InstancingSRTRecorder* m_ptrInsSRT = nullptr;
			CBillboard* m_ptrBillboard = nullptr;
		};

	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="fileName">�e�N�X�`���摜�̃t�@�C���p�X</param>
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		void Init(std::experimental::filesystem::path fileName, int instancingNum = 1);
		//SRV���珉����
		void Init(ID3D11ShaderResourceView* srv, int instancingNum = 1, const wchar_t* identifiers = nullptr, bool isSetIInstancesDataAndShadowPrePost = true);

		//���W�E��]�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			if (m_isIns) {
				m_insModel.SetPos(pos);
				return;
			}
			//m_model.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			if (m_isIns) {
				m_insModel.SetRot(rot);
				return;
			}
			//m_model.SetRot(rot);
		}
		void SetScale(const CVector3& scale) {
			if (m_isIns) {
				m_insModel.SetScale(scale);
				return;
			}
			//m_model.SetScale(scale);
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			//if (m_isIns) {
				return m_insModel.GetPos();
			//}
			//return m_model.GetPos();
		}
		const CQuaternion& GetRot() const {
			//if (m_isIns) {
				return m_insModel.GetRot();
			//}
			//return m_model.GetRot();
		}
		const CVector3& GetScale() const {
			//if (m_isIns) {
				return m_insModel.GetScale();
			//}
			//return m_model.GetScale();
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
				m_insModel.SetIsDraw(isdraw);
			}
			//else {
			//	m_model.SetIsDraw(isdraw);
			//}
		}
		bool GetIsDraw()const {
			if (m_isIns) {
				return m_insModel.GetIsDraw();
			}
			//else {
			//	return m_model.GetIsDraw();
			//}
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
				return m_insModel.GetInstancingModel()->GetModelRender();
			}
			//else {
			//	return m_model;
			//}
		}
		//�C���X�^���V���O���f���̎擾
		GameObj::CInstancingModelRender& GetInstancingModel() {
#ifndef DW_MASTER
			if (!m_isIns) {
				OutputDebugStringA("�y�x���zCBillBoard::GetInstancingModel() �����̃r���{�[�h�̓C���X�^���V���O�`�悶��Ȃ���?\n");
			}
#endif
			return m_insModel;
		}

	private:
		bool m_isInit = false;
		bool m_isIns = false;
		//GameObj::CSkinModelRender m_model;
		GameObj::CInstancingModelRender m_insModel;
	};
//}
}