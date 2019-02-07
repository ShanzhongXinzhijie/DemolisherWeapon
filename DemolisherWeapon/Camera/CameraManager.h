#pragma once

#include "system/GameObject.h"

namespace DemolisherWeapon {

namespace GameObj {

class ICamera : public IQSGameObject{//IGameObject{
public:
	using IQSGameObject::IQSGameObject;
	virtual ~ICamera();

	//メインカメラに設定されているか設定
	//ユーザーは使わないでください
	void SetIsMainCamera(bool isMainCamera) {
		m_isMainCamera = isMainCamera;
	}
private:
	bool m_isMainCamera = false;//メインカメラに設定されているか?

private:
	void UpdateViewMatrix() {
		m_viewMat.MakeLookAt(m_pos, m_target, m_up);
	}
	virtual void UpdateProjMatrix() = 0;

public:
	void PreLoopUpdate()override {
		UpdateOldMatrix();
	};
	void Update()override {
		UpdateMatrix();
	}

	//カメラ(行列)を更新
	void UpdateMatrix() {
		m_change = false;
		UpdateViewMatrix();
		UpdateProjMatrix();
		if (isFirstMatrixUpdate) {
			UpdateOldMatrix();
		}
	};

	void UpdateOldMatrix() {
		m_posOld = m_pos; m_targetOld = m_target; m_upOld = m_up;
		m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
		isFirstMatrixUpdate = false;
	}

	const CMatrix& GetProjMatrix() const { return m_projMat; };
	const CMatrix& GetViewMatrix() const { return m_viewMat; };
	const CMatrix& GetProjMatrixOld() const { return m_projMatOld; };
	CMatrix GetViewMatrixOld() const;

	const CVector3& GetPos() const { return m_pos; }
	CVector3 GetPosOld() const {
		CVector3 move; move.Lerp(MotionBlurScale, m_pos, m_posOld);
		return move;
	}
	const CVector3& GetTarget() const { return m_target; }
	const CVector3& GetUp() const { return m_up; }
	float GetNear() const { return m_near; }
	float GetFar() const{ return m_far; }

	//座標等設定
	void SetPos(const CVector3& v) { m_pos = v; m_change = true; }
	void SetTarget(const CVector3& v) { m_target = v; m_change = true; }
	void SetUp(const CVector3& v) { m_up = v; m_change = true; }

	//遠近設定
	void SetNear(const float v) { m_near = v; m_change = true; }
	void SetFar(const float v) { m_far = v; m_change = true; }

	//ワールド座標からスクリーン座標を計算する
	//const CVector2& screenPos は0.0f～1.0fの範囲。{0.0f,0.0f}で画面左上
	CVector3 CalcScreenPosFromWorldPos(const CVector3& worldPos) ;
	//const CVector2& screenPos をスクリーン座標で取得する版。右下の座標=画面解像度　
	CVector3 CalcScreenPosFromWorldPosScreenPos(const CVector3& worldPos) ;

	//スクリーン座標からワールド座標を計算する
	//const CVector2& screenPos は0.0f～1.0fの範囲。{0.0f,0.0f}で画面左上
	CVector3 CalcWorldPosFromScreenPos(const CVector3& screenPos) ;
	//const CVector2& screenPos をスクリーン座標で指定する版。右下の座標=画面解像度　
	CVector3 CalcWorldPosFromScreenPosScreenPos(const CVector3& screenPos) ;

protected:
	bool m_change = true;//変更点あるか

	CVector3 m_pos = { 0.0f, 0.0f, 500.0f }, m_target = { 0.0f, 0.0f, 0.0f }, m_up = { 0.0f, 1.0f, 0.0f };
	CVector3 m_posOld = { 0.0f, 0.0f, 500.0f }, m_targetOld = { 0.0f, 0.0f, 0.0f }, m_upOld = { 0.0f, 1.0f, 0.0f };
	float m_near = 1.0f, m_far = 1000.0f;

	CMatrix m_projMat, m_viewMat;
	CMatrix m_projMatOld, m_viewMatOld;
	bool isFirstMatrixUpdate = true;
};

//遠近カメラ
class PerspectiveCamera : public ICamera {
public:
	PerspectiveCamera(bool isRegister = true);

	//視野角設定
	void SetViewAngle(const float v) { m_viewAngle = v; m_change = true; }
	void SetViewAngleDeg(const float v) { m_viewAngle = CMath::DegToRad(v); m_change = true;}
	
	//アス比設定
	void SetAspect(const float v) { m_aspect = v; m_change = true;}

private:
	void UpdateProjMatrix()override {
		m_projMat.MakeProjectionMatrix(m_viewAngle, m_aspect, m_near, m_far);
	}

	float m_viewAngle = 3.14f*0.5f;
	float m_aspect = 1280.0f / 720.0f;
};

//平行カメラ
class OrthoCamera : public ICamera {
public:
	//OrthoCamera(bool isRegister = true);
	using ICamera::ICamera;
	virtual ~OrthoCamera() {};

	//幅・高さ設定
	void SetWidth(const float v) { m_width = v; m_change = true;}
	void SetHeight(const float v) { m_height = v; m_change = true;}

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
