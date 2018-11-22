#pragma once

namespace DemolisherWeapon {
namespace GameObj{
namespace Suicider{

class CEffekseer : public IGameObject
{
public:
	CEffekseer();
	CEffekseer(const wchar_t* filePath, const CVector3& pos = CVector3::Zero(), const CQuaternion& rot = CQuaternion::Identity(), const CVector3& scale = CVector3::One());
	~CEffekseer();
	
	void Update() override;


	/*!
	*@brief	解放。
	*/
	void Release();

	/*!
	 *@brief	エフェクトを再生。
	 *@param[in]	filePath		再生するエフェクトのファイルパス。
	 */
	void Play(const wchar_t* filePath, const CVector3& pos = CVector3::Zero(), const CQuaternion& rot = CQuaternion::Identity(), const CVector3& scale = CVector3::One());

	/*!
	 *@brief	座標を設定。
	 */
	void SetPos(const CVector3& pos)
	{
		m_pos = pos;
	}
	/*!
	 *@brief	回転の設定。
	 */
	void SetRot(const CQuaternion& rot)
	{
		m_rot = rot;
	}
	/*!
	 *@brief	拡大率を設定。
	 */
	void SetScale(const CVector3& scale)
	{
		m_scale = scale;
	}
	//座標・回転・拡大の設定
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
		SetPos(pos);
		SetRot(rot);
		SetScale(scale);
	}

	//取得
	const CVector3&		GetPos()const	{ return m_pos; };
	const CQuaternion&  GetRot()const	{ return m_rot; };
	const CVector3&		GetScale()const { return m_scale; };
	void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale)const {
		*pos	= GetPos();
		*rot	= GetRot();
		*scale	= GetScale();
	};

	//再生速度を設定
	void SetSpeed(float speed) {
		GetEngine().GetEffekseer().GetManager()->SetSpeed(m_handle, speed);
	}

	//再生を一時停止
	//paused = trueで一時停止
	void SetPaused(bool paused) {
		GetEngine().GetEffekseer().GetManager()->SetPaused(m_handle, paused);
	}

	//エフェクト全体の色を設定
	void SetColor(const Effekseer::Color& color) {
		GetEngine().GetEffekseer().GetManager()->SetAllColor(m_handle, color);
	}

	//取得
	float GetSpeed()const  { return GetEngine().GetEffekseer().GetManager()->GetSpeed(m_handle); }
	bool  GetPaused()const { return GetEngine().GetEffekseer().GetManager()->GetPaused(m_handle); }

	/*!
	 *@brief	エフェクトが再生中か判定。
	 */
	bool IsPlay() const
	{
		return GetEngine().GetEffekseer().GetManager()->GetShown(m_handle);
	}	

	//ハンドルを取得
	Effekseer::Handle GetHandle()const { return m_handle; }

private:
	Effekseer::Handle m_handle = -1;
	CVector3 m_pos;
	CQuaternion m_rot;
	CVector3 m_scale = CVector3::One();
};

}
}
}