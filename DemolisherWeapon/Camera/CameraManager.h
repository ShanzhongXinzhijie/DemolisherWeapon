#pragma once

#include "system/GameObject.h"

namespace DemolisherWeapon {

namespace GameObj {

class ICamera : public IQSGameObject{//IGameObject{
public:
	using IQSGameObject::IQSGameObject;
	virtual ~ICamera();

	//���C���J�����ɐݒ肳��Ă��邩�ݒ�
	//���[�U�[�͎g��Ȃ��ł�������
	void SetIsMainCamera(bool isMainCamera) {
		m_isMainCamera = isMainCamera;
	}
private:
	bool m_isMainCamera = false;//���C���J�����ɐݒ肳��Ă��邩?

private:
	void UpdateViewMatrix() {
		m_viewMat.MakeLookAt(m_pos, m_target, m_up);
	}
	virtual void UpdateProjMatrix() = 0;

public:
	void PreLoopUpdate()override {
		m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
	};
	void Update()override {
		UpdateMatrix();
	}

	//�J����(�s��)���X�V
	void UpdateMatrix() {
		UpdateViewMatrix();
		UpdateProjMatrix();
		if (isFirstMatrixUpdate) {
			m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
			isFirstMatrixUpdate = false;
		}
	};

	const CMatrix& GetProjMatrix() const { return m_projMat; };
	const CMatrix& GetViewMatrix() const { return m_viewMat; };
	const CMatrix& GetProjMatrixOld() const { return m_projMatOld; };
	const CMatrix& GetViewMatrixOld() const { return m_viewMatOld; };

	const CVector3& GetPos() const { return m_pos; }
	const CVector3& GetTarget() const { return m_target; }
	const CVector3& GetUp() const { return m_up; }
	float GetNear() const { return m_near; }
	float GetFar() const{ return m_far; }

	//���W���ݒ�
	void SetPos(const CVector3& v) { m_pos = v; }
	void SetTarget(const CVector3& v) { m_target = v; }
	void SetUp(const CVector3& v) { m_up = v; }

	//���ߐݒ�
	void SetNear(const float v) { m_near = v; }
	void SetFar(const float v) { m_far = v; }

protected:
	CVector3 m_pos = { 0.0f, 0.0f, 500.0f }, m_target = { 0.0f, 0.0f, 0.0f }, m_up = { 0.0f, 1.0f, 0.0f };
	float m_near = 1.0f, m_far = 1000.0f;

	CMatrix m_projMat, m_viewMat;
	CMatrix m_projMatOld, m_viewMatOld;
	bool isFirstMatrixUpdate = true;
};

//���߃J����
class PerspectiveCamera : public ICamera {
public:
	PerspectiveCamera(bool isRegister = true);

	//����p�ݒ�
	void SetViewAngle(const float v) { m_viewAngle = v; }
	void SetViewAngleDeg(const float v) { m_viewAngle = CMath::DegToRad(v); }
	
	//�A�X��ݒ�
	void SetAspect(const float v) { m_aspect = v; }

private:
	void UpdateProjMatrix()override {
		m_projMat.MakeProjectionMatrix(m_viewAngle, m_aspect, m_near, m_far);
	}

	float m_viewAngle = 3.14f*0.5f;
	float m_aspect = 1280.0f / 720.0f;
};

//���s�J����
class OrthoCamera : public ICamera {
public:
	OrthoCamera(bool isRegister = true);
	virtual ~OrthoCamera() {};

	//���E�����ݒ�
	void SetWidth(const float v) { m_width = v; }
	void SetHeight(const float v) { m_height = v; }

private:
	void UpdateProjMatrix()override {
		m_projMat.MakeOrthoProjectionMatrix(m_width, m_height, m_near, m_far);
	}

	float m_width = 1280.0f, m_height = 720.0f;
};
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
