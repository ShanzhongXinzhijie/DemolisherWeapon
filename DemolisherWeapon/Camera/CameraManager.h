#pragma once

#include "system/GameObject.h"

namespace DemolisherWeapon {

namespace GameObj {

class ICamera : public IQSGameObject{//IGameObject{
public:
	using IQSGameObject::IQSGameObject;
	virtual ~ICamera();

	void PreLoopUpdate()override {
		UpdateOldMatrix();
	};
	void PostUpdate()override {
		UpdateMatrix();
	}
	//void PostLoopUpdate()override {
		//CalcMBlurParameter();
	//}

	//���C���J�����ɐݒ肳��Ă��邩�ݒ�
	//���[�U�[�͎g��Ȃ��ł�������
	void SetIsMainCamera(bool isMainCamera) {
		m_isMainCamera = isMainCamera;
	}
private:
	bool m_isMainCamera = false;//���C���J�����ɐݒ肳��Ă��邩?

private:
	//�s��̍X�V
	void UpdateViewMatrix() {
		m_viewMat.MakeLookAt(m_pos, m_target, m_up);
		m_needUpdateBillboard = true;//�r���{�[�h�s����X�V����K�v������t���O
	}
	virtual void UpdateProjMatrix() = 0;

	//�O��t���[���̏����L�^
	void UpdateOldMatrix() {
		m_posOld = m_pos; m_targetOld = m_target; m_upOld = m_up;
		m_nearOld = m_near, m_farOld = m_far;
		UpdateOldProjParameter();

		m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
		
		isFirstMatrixUpdate = false;
	}
	virtual void UpdateOldProjParameter() = 0;

	//���[�V�����u���[�p�̏����v�Z
	void CalcMBlurParameter() {
		m_posOld.Lerp(MotionBlurScale, m_pos, m_posOld);
		m_targetOld.Lerp(MotionBlurScale, m_target, m_targetOld);
		m_upOld.Lerp(MotionBlurScale, m_up, m_upOld);
		m_nearOld = CMath::Lerp(MotionBlurScale, m_near, m_nearOld);
		m_farOld = CMath::Lerp(MotionBlurScale, m_far, m_farOld);

		m_viewMatOld.MakeLookAt(m_posOld, m_targetOld, m_upOld); //m_viewMatOld.Interpolate(m_viewMat, m_viewMatOld, MotionBlurScale, MotionBlurScale, MotionBlurScale);
		CalcMBlurProjMatrix(m_projMatOld, MotionBlurScale);
	}
	virtual void CalcMBlurProjMatrix(CMatrix& projMOld, float rate) = 0;

	//�r���{�[�h�s��E�N�H�[�^�j�I���̍X�V
	void UpdateBillboard();

public:
	//�J����(�s��)���X�V
	void UpdateMatrix() {
		m_change = false;
		UpdateViewMatrix();
		UpdateProjMatrix();
		if (isFirstMatrixUpdate) {
			UpdateOldMatrix();
		}
	};

	//�s��̎擾
	const CMatrix& GetProjMatrix() const { return m_projMat; };
	const CMatrix& GetViewMatrix() const { return m_viewMat; };
	const CMatrix& GetProjMatrixOld() const { return m_projMatOld; };
	const CMatrix& GetViewMatrixOld() const { return m_viewMatOld; };

	//�p�����[�^�̎擾
	const CVector3& GetPos() const { return m_pos; }
	const CVector3& GetPosOld() const { return m_posOld; }
	const CVector3& GetTarget() const { return m_target; }
	const CVector3& GetUp() const { return m_up; }
	const CVector3& GetFront() {
		if (m_isNeedUpdateFront) { 
			m_isNeedUpdateFront = false; 
			m_front = m_target - m_pos; if (m_front.LengthSq() > FLT_EPSILON) { m_front.Normalize(); }
		} 
		return m_front; 
	}
	float GetNear() const { return m_near; }
	float GetFar() const{ return m_far; }

	virtual float GetFOV()const = 0;
	virtual float GetAspect()const = 0;

	/// <summary>
	/// �w��̋����ɂ����鐍��̃T�C�Y 
	/// </summary>
	/// <param name="z">�J��������̋���</param>
	/// <param name="returnPlaneSize">����̃T�C�Y���Ԃ��Ă���</param>
	virtual void GetFrustumPlaneSize(float z, CVector2& returnPlaneSize)const = 0;

	//���W���ݒ�
	void SetPos(const CVector3& v) { m_pos = v; m_change = true; m_isNeedUpdateFront = true; }
	void SetTarget(const CVector3& v) { m_target = v; m_change = true; m_isNeedUpdateFront = true; }
	void SetUp(const CVector3& v) { m_up = v; m_change = true; }

	//���ߐݒ�
	void SetNear(const float v) { m_near = v; m_change = true; }
	void SetFar(const float v) { m_far = v; m_change = true; }

	//���[���h���W����X�N���[�����W���v�Z����
	//const CVector2& screenPos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
	CVector3 CalcScreenPosFromWorldPos(const CVector3& worldPos) ;
	//const CVector2& screenPos ���X�N���[�����W�Ŏ擾����ŁB�E���̍��W=��ʉ𑜓x�@
	CVector3 CalcScreenPosFromWorldPosScreenPos(const CVector3& worldPos) ;

	//�X�N���[�����W���烏�[���h���W���v�Z����
	//const CVector2& screenPos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
	CVector3 CalcWorldPosFromScreenPos(const CVector3& screenPos) ;
	//const CVector2& screenPos ���X�N���[�����W�Ŏw�肷��ŁB�E���̍��W=��ʉ𑜓x�@
	CVector3 CalcWorldPosFromScreenPosScreenPos(CVector3 screenPos) ;

	/// <summary>
	/// �C���|�X�^�[�N�H�[�^�j�I�����擾
	/// </summary>
	/// <returns></returns>
	CQuaternion GetImposterQuaternion(const CVector3& pos)const;

	/// <summary>
	/// �C���|�X�^�[�s����擾
	/// </summary>
	/// <returns></returns>
	CMatrix GetImposterMatrix(const CVector3& pos)const;

	/// <summary>
	/// �r���{�[�h�N�H�[�^�j�I�����擾
	/// </summary>
	/// <returns></returns>
	CQuaternion GetBillboardQuaternion();

	/// <summary>
	/// �r���{�[�h�s����擾
	/// </summary>
	/// <returns></returns>
	CMatrix GetBillboardMatrix();

protected:
	bool m_change = true;//�ύX�_���邩

	CVector3 m_pos = { 0.0f, 0.0f, 500.0f }, m_target = { 0.0f, 0.0f, 0.0f }, m_up = { 0.0f, 1.0f, 0.0f };
	CVector3 m_posOld = { 0.0f, 0.0f, 500.0f }, m_targetOld = { 0.0f, 0.0f, 0.0f }, m_upOld = { 0.0f, 1.0f, 0.0f };
	CVector3 m_front; bool m_isNeedUpdateFront = true;
	float m_near = 1.0f, m_far = 1000.0f;
	float m_nearOld = 1.0f, m_farOld = 1000.0f;

	CMatrix m_projMat, m_viewMat;
	CMatrix m_projMatOld, m_viewMatOld;
	bool isFirstMatrixUpdate = true;

	//�r���{�[�h�s��E�N�H�[�^�j�I��
	CMatrix m_billboardMat;
	CQuaternion m_billboardQua;
	bool m_needUpdateBillboard = true;//�X�V�̕K�v���邩?
};

//���߃J����
class PerspectiveCamera : public ICamera {
public:
	PerspectiveCamera(bool isRegister = true);

	//����p�ݒ�
	void SetViewAngle(const float v) { m_viewAngle = v; m_change = true; }
	void SetViewAngleDeg(const float v) { m_viewAngle = CMath::DegToRad(v); m_change = true;}
	
	//�A�X��ݒ�
	void SetAspect(const float v) { m_aspect = v; m_change = true;}

	//����p�̎擾
	float GetFOV()const override {
		return m_viewAngle;
	}
	//�A�X�y�N�g��̎擾
	float GetAspect() const override {
		return m_aspect;
	}
	//�w��̋����ɂ����鐍��̃T�C�Y
	void GetFrustumPlaneSize(float z, CVector2& returnPlaneSize)const override {
		returnPlaneSize.y = 2.0f * z * tan(m_viewAngle*0.5f);
		returnPlaneSize.x = m_aspect * returnPlaneSize.y;
	}

private:
	void UpdateProjMatrix()override {
		m_projMat.MakeProjectionMatrix(m_viewAngle, m_aspect, m_near, m_far);
	}
	void UpdateOldProjParameter()override {
		m_viewAngleOld = m_viewAngle;
		m_aspectOld = m_aspect;
	}
	void CalcMBlurProjMatrix(CMatrix& projMOld, float rate)override {
		m_viewAngleOld = CMath::Lerp(rate, m_viewAngle, m_viewAngleOld);
		m_aspectOld = CMath::Lerp(rate, m_aspect, m_aspectOld);
		projMOld.MakeProjectionMatrix(m_viewAngleOld, m_aspectOld, m_nearOld, m_farOld);
	}

	float m_viewAngle = 3.14f*0.5f, m_viewAngleOld = 3.14f*0.5f;
	float m_aspect = 1280.0f / 720.0f, m_aspectOld = 1280.0f / 720.0f;
};

//���s�J����
class OrthoCamera : public ICamera {
public:
	//OrthoCamera(bool isRegister = true);
	using ICamera::ICamera;
	virtual ~OrthoCamera() {};

	//����p�̎擾
	float GetFOV()const override {
		return -1.0f;
	}
	//�A�X�y�N�g��̎擾
	float GetAspect() const override {
		return m_width / m_height;
	}
	//�w��̋����ɂ����鐍��̃T�C�Y
	void GetFrustumPlaneSize(float z, CVector2& returnPlaneSize)const override {
		returnPlaneSize.y = GetHeight();
		returnPlaneSize.x = GetWidth();
	}

	/// <summary>
	/// �v���W�F�N�V�����s������߂邽�߂Ɏg���p�����[�^��I��
	/// </summary>
	/// <param name="iswidthHeight">���ƍ������g���Ȃ�true, �l����ݒ肷��Ȃ�false</param>
	void SetProjMatMode(bool iswidthHeight){
		m_isWidthHeight = iswidthHeight;
	}
	//�l����ݒ�
	void Set4Point(float left, float right, float bottom, float top) {
		m_left = left, m_right = right, m_bottom = bottom, m_top = top;
	}
	//���E�����ݒ�
	void SetWidth(const float v) { m_width = v; m_change = true; }
	void SetHeight(const float v) { m_height = v; m_change = true; }

	//���E�����̎擾
	float GetWidth()const {
		if (m_isWidthHeight) {
			return m_width;
		}
		else {
			return abs(m_left - m_right);
		}
	}
	float GetHeight()const {  
		if (m_isWidthHeight) {
			return m_height;
		}
		else {
			return abs(m_bottom - m_top);
		}
	}

private:
	void UpdateProjMatrix()override {
		if (m_isWidthHeight) {
			m_projMat.MakeOrthoProjectionMatrix(m_width, m_height, m_near, m_far);
		}
		else {
			m_projMat.MakeOrthoProjectionMatrixOffCenter(m_left, m_right, m_bottom, m_top, m_near, m_far);
		}
	}
	void UpdateOldProjParameter()override {
		m_widthOld = m_width;
		m_heightOld = m_height;
	}
	void CalcMBlurProjMatrix(CMatrix& projMOld, float rate)override {
		m_widthOld = CMath::Lerp(rate, m_width, m_widthOld);
		m_heightOld = CMath::Lerp(rate, m_height, m_heightOld);
		projMOld.MakeOrthoProjectionMatrix(m_widthOld, m_heightOld, m_nearOld, m_farOld);
	}

	bool m_isWidthHeight = true;
	float m_width = 1280.0f, m_height = 720.0f;
	float m_widthOld = 1280.0f, m_heightOld = 720.0f;

	float m_left = -640.0f, m_right = 640.0f, m_bottom = -360.0f, m_top = 360.0f;
};
//�Q�[���I�u�W�F�N�g�}�l�[�W���[�ɓo�^���Ȃ��^�C�v�̕��s�J����
class NoRegisterOrthoCamera : public OrthoCamera {
public:
	NoRegisterOrthoCamera() : OrthoCamera(false) {};
};

}

class CameraManager
{
public:

	void SetMainCamera(GameObj::ICamera* c) {
		if (m_mainCamera == c) { return; }

		if (m_mainCamera) { m_mainCamera->SetIsMainCamera(false); }
		m_mainCamera = c;
		if (m_mainCamera) { m_mainCamera->SetIsMainCamera(true); }
	}
	GameObj::ICamera* GetMainCamera() {
		return m_mainCamera;
	}

	std::vector<GameObj::ICamera*>& GetCameraList() {
		return m_cameralist;
	}

private:
	GameObj::ICamera* m_mainCamera = nullptr;
	std::vector<GameObj::ICamera*> m_cameralist;
};

}
