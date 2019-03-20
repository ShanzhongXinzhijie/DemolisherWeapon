#pragma once

namespace DemolisherWeapon {
namespace GameObj{
namespace Suicider{

class CEffekseer : public IGameObject
{
public:
	CEffekseer();
	//*@param[in]	magnification	�ǂݍ��ݎ��̊g�嗦
	CEffekseer(const wchar_t* filePath, const float magnification = 1.0f, const CVector3& pos = CVector3::Zero(), const CQuaternion& rot = CQuaternion::Identity(), const CVector3& scale = CVector3::One());
	~CEffekseer();
	
	void Update() override;

	/*!
	*@brief	����B
	*/
	void Release();

	/*!
	 *@brief	�G�t�F�N�g���Đ��B
	 *@param[in]	filePath		�Đ�����G�t�F�N�g�̃t�@�C���p�X�B
	 *@param[in]	magnification	�ǂݍ��ݎ��̊g�嗦
	 */
	void Play(const wchar_t* filePath, const float magnification = 1.0f, const CVector3& pos = CVector3::Zero(), const CQuaternion& rot = CQuaternion::Identity(), const CVector3& scale = CVector3::One());

	/*!
	 *@brief	���W��ݒ�B
	 */
	void SetPos(const CVector3& pos)
	{
		m_pos = pos;
	}
	/*!
	 *@brief	��]�̐ݒ�B
	 */
	void SetRot(const CQuaternion& rot)
	{
		m_rot = rot;
	}
	/*!
	 *@brief	�g�嗦��ݒ�B
	 */
	void SetScale(const CVector3& scale)
	{
		m_scale = scale;
	}
	//���W�E��]�E�g��̐ݒ�
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
		SetPos(pos);
		SetRot(rot);
		SetScale(scale);
	}

	//�擾
	const CVector3&		GetPos()const	{ return m_pos; };
	const CQuaternion&  GetRot()const	{ return m_rot; };
	const CVector3&		GetScale()const { return m_scale; };
	void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale)const {
		*pos	= GetPos();
		*rot	= GetRot();
		*scale	= GetScale();
	};

	//�Đ����x��ݒ�
	void SetSpeed(float speed) {
		GetEngine().GetEffekseer().GetManager()->SetSpeed(m_handle, speed);
	}

	//�Đ����ꎞ��~
	//paused = true�ňꎞ��~
	void SetPaused(bool paused) {
		GetEngine().GetEffekseer().GetManager()->SetPaused(m_handle, paused);
	}

	//�G�t�F�N�g�S�̂̐F��ݒ�
	void SetColor(const Effekseer::Color& color) {
		GetEngine().GetEffekseer().GetManager()->SetAllColor(m_handle, color);
	}

	//�擾
	float GetSpeed()const  { return GetEngine().GetEffekseer().GetManager()->GetSpeed(m_handle); }
	bool  GetPaused()const { return GetEngine().GetEffekseer().GetManager()->GetPaused(m_handle); }

	/*!
	 *@brief	�G�t�F�N�g���Đ���������B
	 */
	bool IsPlay() const
	{
		return GetEngine().GetEffekseer().GetManager()->GetShown(m_handle);
	}	

	//�n���h�����擾
	Effekseer::Handle GetHandle()const { return m_handle; }

	//�Đ��㎩�E���邩�ǂ����ݒ�
	void SetIsSuicide(bool is) {
		m_isSuicide = is;
	}

private:
	void SetBaseMatrix();

private:
	Effekseer::Handle m_handle = -1;
	CVector3 m_pos;
	CQuaternion m_rot;
	CVector3 m_scale = CVector3::One();

	bool m_isSuicide = true;
};

}
}
}