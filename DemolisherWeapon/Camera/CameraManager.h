#pragma once

#include "system/GameObject.h"

namespace DemolisherWeapon {

namespace GameObj {

class ICamera : public IQSGameObject{
public:
	using IQSGameObject::IQSGameObject;
	virtual ~ICamera();

	void PreLoopUpdate()override {
		UpdateOldMatrix();
	};
	void PostUpdate()override {
		UpdateMatrix();
	}

	//メインカメラに設定されているか設定
	//ユーザーは使わないでください
	void SetIsMainCamera(bool isMainCamera) {
		m_isMainCamera = isMainCamera;
	}

private: bool m_isMainCamera = false;//メインカメラに設定されているか?

private:
	//行列の更新
	void UpdateViewMatrix() {
		m_viewMat.MakeLookAt(m_pos, m_target, m_up);
		m_needUpdateBillboard = true;//ビルボード行列を更新する必要があるフラグ
	}
	virtual void UpdateProjMatrix() = 0;

	//前回フレームの情報を記録
	void UpdateOldMatrix() {
		m_posOld = m_pos; m_targetOld = m_target; m_upOld = m_up;
		m_nearOld = m_near, m_farOld = m_far;
		UpdateOldProjParameter();

		m_projMatOld = m_projMat, m_viewMatOld = m_viewMat;
		
		m_isFirstMatrixUpdate = false;
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

	//視錐台の6平面の計算
	void CalcFrustum6Planes();

	//ビルボード行列・クォータニオンの更新
	void UpdateBillboard();

public:
	//カメラ(行列)を更新
	void UpdateMatrix() {
		m_change = false;
		UpdateViewMatrix();
		UpdateProjMatrix();
		if (m_isFirstMatrixUpdate) {
			UpdateOldMatrix();
		}
		//8平面の更新
		CalcFrustum6Planes();
	};
	//旧行列をリセットする
	void ResetIsFirstMatrixUpdate() { m_isFirstMatrixUpdate = true; }

	//行列の取得
	const CMatrix& GetProjMatrix() const { return m_projMat; };
	const CMatrix& GetViewMatrix() const { return m_viewMat; };
	const CMatrix& GetProjMatrixOld() const { return m_projMatOld; };
	const CMatrix& GetViewMatrixOld() const { return m_viewMatOld; };

	//パラメータの取得
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
	CVector3 GetLeft() {
		return GetFront().GetCross(GetUp()).GetNorm();
	}
	float GetNear() const { return m_near; }
	float GetFar() const{ return m_far; }

	virtual float GetFOV()const = 0;
	virtual float GetAspect()const = 0;

	/// <summary>
	/// 指定の距離における錐台のサイズ 
	/// </summary>
	/// <param name="z">カメラからの距離</param>
	/// <param name="returnPlaneSize">錐台のサイズが返ってくる</param>
	virtual void GetFrustumPlaneSize(float z, CVector2& returnPlaneSize)const = 0;

	enum Frustum6Plane{
		enNearPlane,	//近平面
		enFarPlane,		//遠平面
		enRightPlane,	//右
		enLeftPlane,	//左
		enBottomPlane,	//下
		enTopPlane,		//上
		en6PlaneNum,
	};
	/// <summary>
	/// 視錐台の平面を取得
	/// ※法線は内向き 
	/// </summary>
	/// <param name="ind">どの平面か(0～5)</param>
	/// <returns>視錐台の平面</returns>
	const Plane& GetFrustumPlane(Frustum6Plane ind) {
		if (m_change) { UpdateMatrix(); }//必要あれば行列更新
		return m_planes[ind];
	}

	//座標等設定
	void SetPos(const CVector3& v) { m_pos = v; m_change = true; m_isNeedUpdateFront = true; }
	void SetTarget(const CVector3& v) { m_target = v; m_change = true; m_isNeedUpdateFront = true; }
	void SetUp(const CVector3& v) { m_up = v; m_up.Normalize(); m_change = true; }

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
	CVector3 CalcWorldPosFromScreenPosScreenPos(CVector3 screenPos) ;

	/// <summary>
	/// インポスタークォータニオンを取得
	/// </summary>
	/// <param name="pos">カメラ位置</param>
	CQuaternion GetImposterQuaternion(const CVector3& pos)const;

	/// <summary>
	/// インポスター行列を取得
	/// </summary>
	/// <param name="pos">カメラ位置</param>
	CMatrix GetImposterMatrix(const CVector3& pos)const;

	/// <summary>
	/// ビルボードクォータニオンを取得
	/// </summary>
	CQuaternion GetBillboardQuaternion();

	/// <summary>
	/// ビルボード行列を取得
	/// </summary>
	CMatrix GetBillboardMatrix();

protected:
	bool m_change = true;//変更点あるか

	CVector3 m_pos = { 0.0f, 0.0f, 500.0f }, m_target = { 0.0f, 0.0f, 0.0f }, m_up = { 0.0f, 1.0f, 0.0f };
	CVector3 m_posOld = { 0.0f, 0.0f, 500.0f }, m_targetOld = { 0.0f, 0.0f, 0.0f }, m_upOld = { 0.0f, 1.0f, 0.0f };
	CVector3 m_front; bool m_isNeedUpdateFront = true;
	float m_near = 1.0f, m_far = 1000.0f;
	float m_nearOld = 1.0f, m_farOld = 1000.0f;

	CMatrix m_projMat, m_viewMat;
	CMatrix m_projMatOld, m_viewMatOld;
	bool m_isFirstMatrixUpdate = true;

	Plane m_planes[6];//視錐台の6平面

	//ビルボード行列・クォータニオン
	CMatrix m_billboardMat;
	CQuaternion m_billboardQua;
	bool m_needUpdateBillboard = true;//更新の必要あるか?
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

	//視野角の取得
	float GetFOV()const override {
		return m_viewAngle;
	}
	//アスペクト比の取得
	float GetAspect() const override {
		return m_aspect;
	}
	//指定の距離における錐台のサイズ
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

//平行カメラ
class OrthoCamera : public ICamera {
public:
	//OrthoCamera(bool isRegister = true);
	using ICamera::ICamera;
	virtual ~OrthoCamera() {};

	//視野角の取得
	float GetFOV()const override {
		return -1.0f;
	}
	//アスペクト比の取得
	float GetAspect() const override {
		return m_width / m_height;
	}
	//指定の距離における錐台のサイズ
	void GetFrustumPlaneSize(float z, CVector2& returnPlaneSize)const override {
		returnPlaneSize.y = GetHeight();
		returnPlaneSize.x = GetWidth();
	}

	/// <summary>
	/// プロジェクション行列を求めるために使うパラメータを選択
	/// </summary>
	/// <param name="iswidthHeight">幅と高さを使うならtrue, 四隅を設定するならfalse</param>
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

	//幅・高さの取得
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
//ゲームオブジェクトマネージャーに登録しないタイプの平行カメラ
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
