#pragma once
#include "system/GameObject.h"
#include "Graphic/Model/animation/AnimationController.h"
#include "Graphic/Model/animation/AnimationClip.h"
#include "Graphic/Model/SkinModel.h"

namespace DemolisherWeapon {
namespace GameObj {

class CSkinModelRender : public IQSGameObject{

public:
	CSkinModelRender(bool isRegister = true);

private:
	void PreLoopUpdate()override final;
	void Update()override final;
	void PostUpdate()override final;
	void PostLoopUpdate()override final;	

	void ImNonUpdate() { m_isUpdated = false; };	
	void ImNonUpdateWorldMatrix() { m_isUpdatedWorldMatrix = false; };

public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <param name="animationClips">�A�j���[�V�����N���b�v�z��</param>
	/// <param name="numAnimationClips">�A�j���[�V�����N���b�v��</param>
	/// <param name="fbxUpAxis">fbx�̏㎲�B�f�t�H���g��enFbxUpAxisZ</param>
	/// <param name="fbxCoordinate">fbx�̍��W�n�B�f�t�H���g��enFbxRightHanded</param>
	/// <param name="isUseFlyweightFactory">���f���̃��[�h��FlyweightFactory���g�p���邩</param>
	void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0,
		EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
		EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,
		bool isUseFlyweightFactory = true);

	/*void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0);*/

	void Init(const wchar_t* filePath, EnFbxUpAxis fbxUpAxis, EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded, bool isUseFlyweightFactory = true) {
		Init(filePath, nullptr, 0, fbxUpAxis, fbxCoordinate, isUseFlyweightFactory);
	};

	//���W�E��]�E�g��̐ݒ�
	void SetPos(const CVector3& pos) {
		ImNonUpdateWorldMatrix();
		m_pos = pos;
	}
	void SetRot(const CQuaternion& rot) {
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
		m_rot = rot;
	}
	void SetScale(const CVector3& scale) {
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
		m_scale = scale;
	}
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
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

	//Update�֐������s���邩�ݒ�
	void SetIsEnableUpdate(bool enable) {
		m_isEnableUpdate = enable;
	}

	//�`����s�����ݒ�
	void SetIsDraw(bool flag) { m_model.SetIsDraw(flag); }
	bool GetIsDraw() const { return m_model.GetIsDraw(); }

	//�ʂ𔽓]�����邩�ݒ�
	void SetIsDrawReverse(bool flag) { m_isDrawReverse = flag; }
	bool GetIsDrawReverse()const { return m_isDrawReverse; }

	/// <summary>
	/// �\�t�g�p�[�e�B�N���̎��
	/// </summary>
	enum enSoftParticleMode{
		enOff,//�I�t
		enSoftParticle,//�ʏ�
		enRevSoftParticle,//�t
	};
	/// <summary>
	/// �|�X�g�h���[�`���������
	/// </summary>
	/// <param name="blendMode">�u�����h���[�h</param>
	/// <param name="isPMA">���f������Z�ς݃A���t�@��?</param>
	/// <param name="isSoftParticle">�\�t�g�p�[�e�B�N����L�������邩</param>
	/// <param name="softParticleArea">�\�t�g�p�[�e�B�N�����������鋗���͈�(��0�Ńf�t�H���g)</param>
	void InitPostDraw(PostDrawModelRender::enBlendMode blendMode, bool isPMA = false, enSoftParticleMode isSoftParticle = enOff, float softParticleArea = -1.0f);

	//�V���h�E�}�b�v�ւ̕`����s�����ݒ�
	void SetIsShadowCaster(bool flag) { m_isShadowCaster = flag; }
	//�V���h�E�}�b�v�̕`�掞�ɖʂ𔽓]�����邩�ݒ�
	void SetIsShadowDrawReverse(bool flag) { m_isShadowDrawReverse = flag; }
	bool GetIsShadowDrawReverse()const { return m_isShadowDrawReverse; }

	//�o�E���f�B���O�{�b�N�X��\�����邩�ݒ�
	void SetIsDrawBoundingBox(bool enable) { m_isDrawBoundingBox = enable; }

	//��ԉ��ɕ`�悷�邩�ݒ�
	void SetIsMostDepth(bool enable) {
		if (enable) {
			m_model.SetRasterizerState(m_mostDepthRSCw, m_mostDepthRSCCw, m_mostDepthRSNone);
			m_model.SetDepthBias(1.0f);
		}
		else {
			m_model.SetRasterizerState(nullptr, nullptr, nullptr);
			m_model.SetDepthBias(0.0f);
		}
	}

	//�`�揇��ݒ�(���l���傫���قǌ�ɕ`�悳���)
	void SetDrawPriority(int prio) { m_priority = prio; }
	int GetDrawPriority()const { return m_priority; }

	//���[���h�s����X�V(�X�P���g�����X�V�����)
	void UpdateWorldMatrix(bool refreshOldPos = false) {
		if (m_isSetRotOrScale) {
			//���[���h�s��X�V
			m_model.UpdateWorldMatrix(m_pos, m_rot, m_scale, refreshOldPos);
			m_isSetRotOrScale = false;
		}
		else {
			//���[���h�s��̕��s�ړ������X�V
			m_model.UpdateWorldMatrixTranslation(m_pos, refreshOldPos);
		}
		m_isUpdatedWorldMatrix = true;
	}
	//���[���h�s������t���b�V��
	void RefreshWorldMatrix() {
		ImNonUpdate();
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
		m_isRefreshMode = true;
		Update();
		m_isRefreshMode = false;
	}

private:
	bool m_isInit = false;
	bool m_isRefreshMode = false;

	int m_priority = DRAW_PRIORITY_DEFAULT;//�`�揇
	
	SkinModel m_model;
	CVector3		m_pos;
	CQuaternion		m_rot;
	CVector3		m_scale = CVector3::One();

	AnimationController m_animCon;

	bool m_isEnableUpdate = true;//Update�����s���邩?

	//bool m_isDraw = true;				//�\�����邩
	bool m_isDrawReverse = false;		//�ʂ𔽓]���ĕ`�悷�邩
	bool m_isShadowCaster = true;		//�V���h�E�}�b�v�ɏ������ނ�
	bool m_isShadowDrawReverse = true;	//�V���h�E�}�b�v�`�掞�ɖʂ𔽓]�����邩?
	bool m_isDrawBoundingBox = false;	//�o�E���f�B���O�{�b�N�X��\�����邩?
	bool m_isPostDraw = false;			//�|�X�g�h���[��?
	PostDrawModelRender::enBlendMode m_postDrawBlendMode = PostDrawModelRender::enBlendMode::enAlpha;//�|�X�g�h���[�̃u�����h���[�h

	bool m_isUpdated = false;			//�A�b�v�f�[�g�ς݂�?
	bool m_isUpdatedWorldMatrix = false;//���[���h�s��X�V�ς݂�?
	bool m_isSetRotOrScale = true;		//��]�܂��͊g���ݒ肵����?

	bool m_animUpdating = false;

	//�f�ނ̖��V�F�[�_
	Shader m_psSozaiNoAzi, m_psSozaiNoAziTex;
	bool m_loadedShaderSNA = false;			//�f�ނ̖��V�F�[�_�̓��[�h�ς݂�?
	bool m_shaderSNAIsConvertPMA = false;	//�f�ނ̖��V�F�[�_�͏�Z�ς݃A���t�@�ϊ��ł�?
	enSoftParticleMode m_shaderSNAIsSoftParticle = enOff;	//�f�ނ̖��V�F�[�_�̓\�t�g�p�[�e�B�N���ł�?

	//std::unique_ptr<ShadowMapRender::IPrePost> m_shadowMapPrePost;

	static ID3D11RasterizerState* m_mostDepthRSCw;
	static ID3D11RasterizerState* m_mostDepthRSCCw;
	static ID3D11RasterizerState* m_mostDepthRSNone;
};

}
}