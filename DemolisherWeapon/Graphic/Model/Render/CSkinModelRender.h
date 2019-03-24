#pragma once
#include "system/GameObject.h"
#include "../animation/AnimationController.h"
#include "../animation/AnimationClip.h"

namespace DemolisherWeapon {
namespace GameObj {

class CSkinModelRender : public IQSGameObject{

public:
	CSkinModelRender();
	~CSkinModelRender();

private:
	void PreLoopUpdate()override final;
	void Update()override final;
	void PostUpdate()override final;
	void PostLoopUpdate()override final;	

	void ImNonUpdate() { m_isUpdated = false; };	
	void ImNonUpdateWorldMatrix() { m_isUpdatedWorldMatrix = false; };

public:

	//������
	void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0,
		EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
		EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded);

	void Init(const wchar_t* filePath, EnFbxUpAxis fbxUpAxis, EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded) {
		Init(filePath, nullptr, 0, fbxUpAxis, fbxCoordinate);
	};

	//���W�E��]�E�g��̐ݒ�
	void SetPos(const CVector3& pos) {
		ImNonUpdateWorldMatrix();
		m_pos = pos;
	}
	void SetRot(const CQuaternion& rot) {
		ImNonUpdateWorldMatrix();
		m_rot = rot;
	}
	void SetScale(const CVector3& scale) {
		ImNonUpdateWorldMatrix();
		m_scale = scale;
	}
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
		ImNonUpdateWorldMatrix();
		SetPos(pos);
		SetRot(rot);
		SetScale(scale);
	}
	//���W�E��]�E�g��̎擾
	const CVector3& GetPos() const{
		return m_pos ;
	}
	const CQuaternion& GetRot() const {
		return m_rot;
	}
	const CVector3& GetScale() const {
		return m_scale;
	}
	void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const{
		*pos = GetPos();
		*rot = GetRot();
		*scale = GetScale();
	}

	//�{�[���̍��W�E��]�E�g��̎擾
	void CalcWorldTRS(int boneindex, CVector3& trans, CQuaternion& rot, CVector3& scale) {
		Update();
		GetSkinModel().GetSkeleton().GetBone(boneindex)->CalcWorldTRS(trans, rot, scale);		
	}
	const CVector3& GetBonePos(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetPosition();
	};
	const CQuaternion& GetBoneRot(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetRotation();
	};
	const CVector3& GetBoneScale(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetScale();
	};

	//�{�[���ԍ��𖼑O�Ō���
	int FindBoneID(const wchar_t* boneName) {
		return GetSkinModel().GetSkeleton().FindBoneID(boneName);
	}

	//�{�[���̃|�C���^�𖼑O�Ō���
	Bone* FindBone(const wchar_t* boneName) {
		return GetSkinModel().FindBone(boneName);
	}

	//�X�L�����f���̎擾
	SkinModel& GetSkinModel() {
		//ImNonUpdate();
		return m_model;
	}
	const SkinModel& GetSkinModel() const
	{
		return m_model;
	}

	//�A�j���[�V�����R���g���[���[�̎擾
	AnimationController& GetAnimCon() {
		//ImNonUpdate();
		return m_animCon;
	}	

	//�`����s�����ݒ�
	void SetIsDraw(bool flag) {
		m_isDraw = flag;
	}
	bool GetIsDraw() const{
		return m_isDraw;
	}
	//�V���h�E�}�b�v�ւ̕`����s�����ݒ�
	void SetIsShadowCaster(bool flag){
		m_isShadowCaster = flag;
	}

	//��ԉ��ɕ`�悷�邩�ݒ�
	void SetIsMostDepth(bool enable) {
		if (enable) {
			m_model.SetRasterizerState(m_mostDepthRSCw, m_mostDepthRSCCw);
			m_model.SetDepthBias(1.0f);
		}
		else {
			m_model.SetRasterizerState(nullptr, nullptr);
			m_model.SetDepthBias(0.0f);
		}
	}

	//�`�揇��ݒ�(���l���傫���قǌ�ɕ`�悳���)
	void SetDrawPriority(int prio) {
		m_priority = prio;
	}
	int GetDrawPriority()const {
		return m_priority;
	}

	//���[���h�s����X�V
	void UpdateWorldMatrix(bool reflesh = false) {
		m_model.UpdateWorldMatrix(m_pos, m_rot, m_scale, reflesh);
		m_isUpdatedWorldMatrix = true;
	}
	//���[���h�s������t���b�V��
	void RefreshWorldMatrix() {
		ImNonUpdate();
		ImNonUpdateWorldMatrix();
		m_isRefreshMode = true;
		Update();
		m_isRefreshMode = false;
	}

private:
	bool m_isInit = false;
	bool m_isRefreshMode = false;

	int m_priority = DRAW_PRIORITY_DEFAULT;
	
	SkinModel m_model;
	CVector3		m_pos;
	CQuaternion		m_rot;
	CVector3		m_scale = CVector3::One();

	AnimationController m_animCon;

	bool m_isDraw = true;//�\�����邩
	bool m_isShadowCaster = true;//�V���h�E�}�b�v�ɏ������ނ�

	bool m_isUpdated = false;			//�A�b�v�f�[�g�ς݂�?
	bool m_isUpdatedWorldMatrix = false;//���[���h�s��X�V�ς݂�?

	bool m_animUpdating = false;

	static ID3D11RasterizerState* m_mostDepthRSCw;
	static ID3D11RasterizerState* m_mostDepthRSCCw;
};

}
}