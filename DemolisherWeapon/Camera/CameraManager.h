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

	//前回フレームの情報を記録
	void UpdateOldMatrix() {
		m_posOld = m_pos; m_targetOld = m_target; m_upOld = m_up;
		m_nearOld = m_near, m_farOld = m_far;
		UpdateOldProjParameter();

		m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
		
		isFirstMatrixUpdate = false;
	}
	virtual void UpdateOldProjParameter() = 0;

	//モーションブラー用の情報を計算
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

public:
	void PreLoopUpdate()override {
		UpdateOldMatrix();
	};
	void Update()override {
		UpdateMatrix();
	}
	//void PostLoopUpdate()override {
		//CalcMBlurParameter();
	//}

	//カメラ(行列)を更新
	void UpdateMatrix() {
		m_change = false;
		UpdateViewMatrix();
		UpdateProjMatrix();
		if (isFirstMatrixUpdate) {
			UpdateOldMatrix();
		}
	};

	const CMatrix& GetProjMatrix() const { return m_projMat; };
	const CMatrix& GetViewMatrix() const { return m_viewMat; };
	const CMatrix& GetProjMatrixOld() const { return m_projMatOld; };
	const CMatrix& GetViewMatrixOld() const { return m_viewMatOld; };

	const CVector3& GetPos() const { return m_pos; }
	const CVector3& GetPosOld() const { return m_posOld; }
	const CVector3& GetTarget() const { return m_target; }
	const CVector3& GetUp() const { return m_up; }
	float GetNear() const { return m_near; }
	float GetFar() const{ return m_far; }

	virtual float GetFOV()const = 0;
	virtual float GetAspect()const = 0;

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
	float m_nearOld = 1.0f, m_farOld = 1000.0f;

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

	float GetFOV()const override {
		return m_viewAngle;
	}
	float GetAspect() const override {
		return m_aspect;
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

//平行カメラ
class OrthoCamera : public ICamera {
public:
	//OrthoCamera(bool isRegister = true);
	using ICamera::ICamera;
	virtual ~OrthoCamera() {};

	float GetFOV()const override {
		return -1.0f;
	}
	float GetAspect() const override {
		return m_width / m_height;
	}

	//プロジェクション行列を求めるために使うパラメータを選択
	void SetProjMatMode(bool iswidthHeight){
		m_isWidthHeight = iswidthHeight;
	}
	//四隅を設定
	void Set4Point(float left, float right, float bottom, float top) {
		m_left = left, m_right = right, m_bottom = bottom, m_top = top;
	}
	//幅・高さ設定
	void SetWidth(const float v) { m_width = v; m_change = true; }
	void SetHeight(const float v) { m_height = v; m_change = true; }

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
